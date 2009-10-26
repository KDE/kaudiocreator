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
#include <QTreeView>
#include <QProgressBar>
#include <QStandardItemModel>

#include <QtDebug>

#include <kconfig.h>
#include <kglobal.h>
#include <kdebug.h>
#include <kiconloader.h>
#include <kmessagebox.h>
#include <kurl.h>
#include <kstandarddirs.h>
#include <knotification.h>

#include "job.h"
#include "jobqueimp.h"
#include "jobdelegate.h"
#include "defs.h"
#include "prefs.h"

/**
 * Constructor, set up signals.
 * @param parent - parent widget
 * @param name - widget name
 */
JobQueImp::JobQueImp(QWidget* parent) :
		JobQue(parent), currentId(0)
{
    jobModel = new QStandardItemModel(0, 3, this);
    jobModel->setHorizontalHeaderLabels(QStringList() << i18n("Job") << i18n("Progress") << i18n("Description"));
    jobView->setModel(jobModel);
    jobView->setItemDelegate(new JobDelegate());

	connect(removeSelected, SIGNAL(clicked()), this, SLOT( removeSelectedJob()));
	connect(removeAll, SIGNAL(clicked()), this, SLOT(removeAllJobs()));
	connect(removeDoneJobs, SIGNAL(clicked()), this, SLOT(clearDoneJobs()));
}

/**
 * Add a new job to the que
 * @param id the id of the job.
 * @param name the name of the job.
 */
void JobQueImp::addJob(Job *job, const QString &name)
{
    if(!job)
        return;

    QList<QStandardItem *> jobItems = QList<QStandardItem *>();
    job->id = ++currentId;
    QStandardItem *jobItem = new QStandardItem(QString::number(currentId));
    jobItem->setEditable(FALSE);
    jobItems << jobItem;
    QStandardItem *progressItem = new QStandardItem();
    progressItem->setData(JOB_QUEUED, JobState);
    progressItem->setData(job->id, JobId);
    progressItem->setData(0, PercentDone);
    progressItem->setEditable(FALSE);
    jobItems << progressItem;
    QStandardItem *descriptionItem = new QStandardItem(name);
    descriptionItem->setEditable(FALSE);
    jobItems << descriptionItem;
    jobModel->appendRow(jobItems);
    queLabel->setText(i18n("Number of jobs in the queue: %1", jobModel->rowCount()));
}

/**
 * Locate the job and update the progress.
 * @param id the id of the job to update
 * @param progress the new progress of the job.
 */
void JobQueImp::updateProgress(int id, int progress)
{
    QList<QStandardItem *> jobs = jobModel->findItems(QString::number(id), Qt::MatchExactly, HEADER_JOB);

	if (jobs.isEmpty()) {
		qDebug() << "JobQueImp::updateProgress An update was received about a job, "
				  "but the job couldn't be found: " << id;
		return;
	}

    QModelIndex progressIndex = jobs.at(0)->index();
    progressIndex = progressIndex.sibling(progressIndex.row(), HEADER_PROGRESS);
    QStandardItem *currentItem = jobModel->itemFromIndex(progressIndex);
	// Only update the % if it changed.
	if (progress > 0 && currentItem->data(PercentDone).toInt() == progress)
		return;

	if (progress >= 0 && progress < JOB_COMPLETED) {
        currentItem->setData(progress, PercentDone);
        if  (currentItem->data(JobState).toInt() != JOB_PROGRESSING) {
            currentItem->setData(JOB_PROGRESSING, JobState);
        }
    } else if (progress == JOB_ERROR) {
        currentItem->setData(JOB_ERROR, JobState);
    } else if (progress == JOB_STARTED) {
        currentItem->setData(JOB_STARTED, JobState);
    } else if (progress == JOB_COMPLETED) {
        currentItem->setData(i18n("Done"), Qt::DisplayRole);
        currentItem->setData(JOB_COMPLETED, JobState);
        currentItem->setData(100, PercentDone);
		if (Prefs::removeCompletedJobs()){
			removeJob(currentItem, FALSE);
			return;
		}
	}

	if (numberOfJobsNotFinished() == 0)
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
bool JobQueImp::removeJob(QStandardItem *item, bool kill, bool prompt)
{
	if (!item)
		return false;

	if (item->data(JobState).toInt() != JOB_COMPLETED &&
        (prompt && KMessageBox::questionYesNo(this, i18n("KAudioCreator has not finished %1. Remove anyway?", item->data(Qt::DisplayRole).toString()), i18n("Unfinished Job in Queue"), KStandardGuiItem::del(), KGuiItem(i18n("Keep")))
		  == KMessageBox::No))
		return false;

	// "Thread" safe
	if(!item)
		return false;

	if(kill)
		emit (removeJob(item->data(Qt::DisplayRole).toInt()));

	jobModel->removeRow(item->row(), QModelIndex());

	// See if the Que needs to be updated...
	if (jobModel->rowCount() == 0) {
		queLabel->setText(i18n("No jobs are in the queue"));
		currentId = 0; // no jobs left so we can reset the count
	} else {
		queLabel->setText(i18n("Number of jobs in the queue: %1", jobModel->rowCount()));
    }

	return true;
}

/**
 * Remove selected Jobs
 */
void JobQueImp::removeSelectedJob()
{
    QModelIndex index;
    QModelIndexList items = jobView->selectionModel()->selectedRows(0);
    foreach (index, items) {
        jobModel->removeRow(index.row(), QModelIndex());
    }
}

/**
 * Remove all of the jobs in the list.
 */
void JobQueImp::removeAllJobs()
{
	// First determine if there are jobs not finished and prompt once here
	bool finished=true;
    int rows = jobModel->rowCount();
    for (int r = 0; r < rows; ++r) {
        QModelIndex index = jobModel->index(r, HEADER_PROGRESS, QModelIndex());
        if (jobModel->data(index, JobState).toInt() != JOB_COMPLETED) {
            finished = false;
            break;
        }
	}

	if (!finished) {
		if (KMessageBox::questionYesNo(this, i18n("KAudioCreator has not finished all of the jobs. Remove them anyway?"), i18n("Unfinished Job in Queue"), KStandardGuiItem::del(), KGuiItem(i18n("Keep")))
			== KMessageBox::No )
		return;
	}

	while (jobModel->rowCount()) {
		removeJob(jobModel->item(0, 0), true, false);
	}
}

/**
 * Remove any jobs that are in the list that are done.
 */
void JobQueImp::clearDoneJobs()
{
    QList<QStandardItem *> jobs = jobModel->findItems(i18n("Done"), Qt::MatchExactly, HEADER_PROGRESS);
    
    foreach (QStandardItem *item, jobs) {
			emit (removeJob(item->data(JobId).toInt()));
			jobModel->removeRow(item->row(), QModelIndex());
	}

	if (jobModel->rowCount() == 0) {
		queLabel->setText(i18n("No jobs are in the queue"));
		currentId = 0; // no jobs left so we can reset the count
	} else {
		queLabel->setText(i18n("Number of jobs in the queue: %1", jobModel->rowCount()));
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
    int rows = jobModel->rowCount();
    for (int r = 0; r < rows; ++r) {
        QModelIndex index = jobModel->index(r, HEADER_PROGRESS, QModelIndex());
        if (jobModel->data(index, JobState).toInt() != JOB_COMPLETED) {
			++totalJobsToDo;
        }
	}
	return totalJobsToDo;
}

#include "jobqueimp.moc"

