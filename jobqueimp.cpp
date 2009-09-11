/**
 * Copyright (C) 2003-2005 Benjamin C Meyer (ben at meyerhome dot net)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include <QPushButton>
#include <QPainter>
#include <QFile>
#include <QTreeWidget>
#include <QTreeWidgetItemIterator>
#include <QProgressBar>

#include <kconfig.h>
#include <kglobal.h>
#include <kdebug.h>
#include <kiconloader.h>
#include <kmessagebox.h>
#include <kurl.h>
#include <kstandarddirs.h>
#include <knotification.h>

#include "jobqueimp.h"
#include "job.h"
#include "prefs.h"

#define HEADER_JOB 0
#define HEADER_PROGRESS 1
#define HEADER_DESCRIPTION 2
#define ICON_LOC HEADER_DESCRIPTION

#define DEFAULT_HIGHEST_NUMBER 9

/**
 * Constructor, set up signals.
 * @param parent - parent widget
 * @param name - widget name
 */
JobQueImp::JobQueImp( QWidget* parent) :
		JobQue(parent),highestNumber(DEFAULT_HIGHEST_NUMBER), currentId(0){
	connect(removeSelected,SIGNAL(clicked()), this, SLOT( removeSelectedJob()));
	connect(removeAll, SIGNAL(clicked()), this, SLOT(removeAllJobs()));
	connect(removeDoneJobs, SIGNAL(clicked()), this, SLOT(clearDoneJobs()));
}

/**
 * Return a buffer of "000" so that new, updated jobs strings will be able to
 * sort via the columns.
 * Based upon a highest number that is kept.
 * @param number the number to fill out.
 */
QString JobQueImp::getStringFromNumber(int number){
	if (number > highestNumber) {
		int diff = QString("%1").arg(number).length() - QString("%1").arg(highestNumber).length();
		highestNumber = number;
		if(diff > 0){
			// We have to update all of the cells.
			int jobCount = todoQue->topLevelItemCount();
			for (int j = 0; j < jobCount; ++j) {
				QTreeWidgetItem *item = todoQue->topLevelItem(j);
				item->setText(HEADER_JOB, '0' + item->text(HEADER_JOB));
			}
		}
	}

	QString buffer = "";
	uint newLength = QString("%1").arg(highestNumber).length() - QString("%1").arg(number).length();
	for(uint i=0; i < newLength; i++)
		buffer += '0';
	return buffer;
}

/**
 * Add a new job to the que
 * @param id the id of the job.
 * @param name the name of the job.
 */
void JobQueImp::addJob(Job *job, const QString &name) {
	if(!job)
		return;
	job->id = ++currentId;
	QTreeWidgetItem *newItem = new QTreeWidgetItem();
	newItem->setText(HEADER_JOB, QString("%1%2").arg(getStringFromNumber(currentId)).arg(currentId));
	newItem->setIcon(ICON_LOC, KIcon("media-playback-pause"));
	newItem->setText(HEADER_DESCRIPTION, name);
    newItem->setData(HEADER_JOB, PercentDone, QVariant(0));
    newItem->setData(HEADER_JOB, Progressing, QVariant(false));
    newItem->setData(HEADER_JOB, Finished, QVariant(false));
	todoQue->addTopLevelItem(newItem);
	QProgressBar *pg = new QProgressBar();
	pg->setValue(0);
	todoQue->setItemWidget(newItem, HEADER_PROGRESS, pg);
	queLabel->setText(i18n("Number of jobs in the queue: %1", todoQue->topLevelItemCount()));
}

/**
 * Locate the job and update the progress.
 * @param id the id of the job to update
 * @param progress the new progress of the job.
 */
void JobQueImp::updateProgress(int id, int progress)
{
	int currentJobCount = numberOfJobsNotFinished();
	QString buffer = getStringFromNumber(id);
	buffer += QString("%1").arg(id);

	// Find the current item
	QTreeWidgetItem *currentItem = 0;
	QTreeWidgetItemIterator it(todoQue);
	while (*it) {
		if ((*it)->text(HEADER_JOB) == buffer) {
			currentItem = *it;
			break;
		}
		++it;
	}

	if (!currentItem) {
		kDebug() << "JobQueImp::updateProgress An update was received about a job, "
				  "but the job couldn't be found: " << id << endl;
		return;
	}

	// Only update the % if it changed.
	if (progress > 0 && currentItem->data(HEADER_JOB, PercentDone).toInt() == progress)
		return;

	currentItem->setData(HEADER_JOB, PercentDone, QVariant(progress));
	QProgressBar *pg = (QProgressBar *)todoQue->itemWidget(currentItem, HEADER_PROGRESS);

	// Update the icon if needed
	if (progress > 0 && progress < 100) {
        if  (!currentItem->data(HEADER_JOB, Progressing).toBool()) {
            currentItem->setIcon(ICON_LOC, KIcon("system-run"));
            currentItem->setData(HEADER_JOB, Progressing, QVariant(true));
        }
		if (pg->maximum() == 0) pg->setMaximum(100);
		pg->setValue(progress);
	} else if (progress == 0) {
		if  (!currentItem->data(HEADER_JOB, Progressing).toBool()) {
			currentItem->setIcon(ICON_LOC, KIcon("system-run"));
			currentItem->setData(HEADER_JOB, Progressing, QVariant(true));
		}
		pg->setMinimum(0);
		pg->setMaximum(0);
	} else if(progress == -1) {
		currentItem->setIcon(ICON_LOC, KIcon("dialog-cancel"));
        currentItem->setData(HEADER_JOB, Finished, QVariant(true));
		pg->setMaximum(100);
		pg->setValue(0);
	} else if (progress == 100) {
        currentItem->setIcon(ICON_LOC, KIcon("dialog-ok"));
        currentItem->setData(HEADER_JOB, Finished, QVariant(true));
        pg->setValue(progress);
		// Remove the job if requested.
		if(Prefs::removeCompletedJobs()){
			removeJob(currentItem, false);
			return;
		}
	}

	if (currentJobCount > 0 && numberOfJobsNotFinished() == 0)
		KNotification::event("no jobs left");
}

/**
 * Remove job listed in item
 * @param item to remove. Note that it WILL be deleted and set to NULL.
 * @param kill kill the actual job/process. A bool here because this CAN cause
 *        a race condition when the encoder is 100%, but hasn't exited.
 * @param prompt the user if the job isn't finished
 * @return bool if remove was successful or not.
 */
bool JobQueImp::removeJob(QTreeWidgetItem *item, bool kill, bool prompt)
{
	if (!item)
		return false;

	if (!item->data(HEADER_JOB, Finished).toBool() &&
        (prompt && KMessageBox::questionYesNo(this, i18n("KAudioCreator has not finished %1. Remove anyway?", item->text(HEADER_DESCRIPTION)), i18n("Unfinished Job in Queue"), KStandardGuiItem::del(), KGuiItem(i18n("Keep")))
		  == KMessageBox::No ))
		return false;

	// "Thread" safe
	if(!item)
		return false;

	if(kill)
		emit (removeJob(item->text(HEADER_JOB).toInt()));

	todoQue->invisibleRootItem()->removeChild(item);
	delete(item);

	// See if the Que needs to be updated...
	if (todoQue->topLevelItemCount() == 0) {
		queLabel->setText(i18n("No jobs are in the queue"));
		highestNumber = DEFAULT_HIGHEST_NUMBER;
		currentId = 0;
	}
	else
		queLabel->setText(i18n("Number of jobs in the queue: %1", todoQue->topLevelItemCount()));
	return true;
}

/**
 * Remove selected Jobs
 */
void JobQueImp::removeSelectedJob(){
	QList<QTreeWidgetItem *> selected = todoQue->selectedItems();
	foreach (QTreeWidgetItem *sel, selected) {
		removeJob(sel);
	}
}

/**
 * Remove all of the jobs in the list.
 */
void JobQueImp::removeAllJobs(){
	// First determine if there are jobs not finished and prompt once here
	bool finished=true;
	QTreeWidgetItemIterator it(todoQue);
	while (*it) {
		if (!(*it)->data(HEADER_JOB, Finished).toBool()) {
			finished = false;
            break;
        }
		++it;
	}

	if (!finished) {
		if (KMessageBox::questionYesNo(this, i18n("KAudioCreator has not finished all of the jobs. Remove them anyway?"), i18n("Unfinished Job in Queue"), KStandardGuiItem::del(), KGuiItem(i18n("Keep")))
			== KMessageBox::No )
		return;
	}

	while (todoQue->topLevelItemCount()) {
		removeJob(todoQue->topLevelItem(0), true, false);
	}
}

/**
 * Remove any jobs that are in the list that are done.
 */
void JobQueImp::clearDoneJobs()
{
	QTreeWidgetItemIterator it(todoQue);
	while (*it) {
		if ((*it)->data(HEADER_JOB, PercentDone).toInt() == 100) {
			emit (removeJob((*it)->text(HEADER_JOB).toInt()));
			todoQue->invisibleRootItem()->removeChild(*it);
		}
		++it;
	}

	if (todoQue->topLevelItemCount() == 0) {
		queLabel->setText(i18n("No jobs are in the queue"));
		highestNumber = DEFAULT_HIGHEST_NUMBER;
		currentId = 0;
	} else {
		queLabel->setText(i18n("Number of jobs in the queue: %1", todoQue->topLevelItemCount()));
    }
}

/**
 * Return the number of jobs in the que that don't have 100% or Error in the
 * Progress column
 * @return the number of jobs that are in the que that haven't been finished.
 */
int JobQueImp::numberOfJobsNotFinished()
{
	int totalJobsToDo = 0;
	QTreeWidgetItemIterator it(todoQue);
	while (*it) {
		if (!(*it)->data(HEADER_JOB, Finished).toBool())
			++totalJobsToDo;
		++it;
	}
	return totalJobsToDo;
}

#include "jobqueimp.moc"

