/**
 * This file is part of the KAudioCreator package
 * Copyright (C) 2003 Benjamin C Meyer (ben+kaudiocreator at meyerhome dot net)
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
 * the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include "jobqueimp.h"
#include "job.h"
#include "prefs.h"
#include <qpushbutton.h>
#include <qlabel.h>
#include <qpainter.h>
#include <kconfig.h>
#include <kglobal.h>
#include <kdebug.h>
#include <kiconloader.h>
#include <kmessagebox.h>
#include <kurl.h>
#include <qfile.h>
#include <qregexp.h>
#include <qfileinfo.h>
#include <kstandarddirs.h>
#include <knotifyclient.h>
#include <qdir.h>

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
JobQueImp::JobQueImp( QWidget* parent, const char* name) :
		JobQue(parent,name),highestNumber(DEFAULT_HIGHEST_NUMBER), currentId(0){
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
	if(number > highestNumber){
		int diff = QString("%1").arg(number).length() - QString("%1").arg(highestNumber).length();
		highestNumber = number;
		if(diff > 0){
			// We have to update all of the cells.
			QueListViewItem * currentItem = (QueListViewItem*)todoQue->firstChild();
			while( currentItem != NULL ){
				currentItem->setText(HEADER_JOB, "0" + currentItem->text(HEADER_JOB));
				currentItem = (QueListViewItem*)currentItem->itemBelow();
			}
		}
	}

	QString buffer = "";
	uint newLength = QString("%1").arg(highestNumber).length() - QString("%1").arg(number).length();
	for(uint i=0; i < newLength; i++)
		buffer += "0";
	return buffer;
}

/**
 * Add a new job to the que
 * @param id the id of the job.
 * @param name the name of the job.
 */
void JobQueImp::addJob(Job*job, const QString &name ){
	if(!job)
		return;
	job->id = ++currentId;
	QueListViewItem *currentItem = new QueListViewItem(todoQue, QString("%1%2").arg(getStringFromNumber(currentId)).arg(currentId), "0", name);
	currentItem->setPixmap(ICON_LOC, SmallIcon("player_pause", currentItem->height()-2));
	queLabel->setText(i18n("Number of jobs in the queue: %1").arg(todoQue->childCount()));
}

/**
 * Locate the job and update the progress.
 * @param id the id of the job to update
 * @param progress the new progress of the job.
 */
void JobQueImp::updateProgress(int id, int progress){
	int currentJobCount = numberOfJobsNotFinished();
	QueListViewItem * currentItem = (QueListViewItem*)todoQue->firstChild();
	QString buffer = getStringFromNumber(id);
	buffer += QString("%1").arg(id);

	// Find the current item
	while( currentItem != NULL ){
		if(currentItem->text(HEADER_JOB) == buffer)
			break;
		currentItem = (QueListViewItem*)currentItem->nextSibling();
	}
	if( !currentItem ){
		kdDebug(60002) << "JobQueImp::updateProgress An update was received about a job, but the job couldn't be found: " << id << endl;
		return;
	}

	// Only update the % if it changed.
	if(currentItem->percentDone == progress)
		return;

	currentItem->percentDone = progress;
	currentItem->repaint();

	// Update the icon if needed
	if(progress > 0 && progress < 100 && !currentItem->progressing ){
		currentItem->setPixmap(ICON_LOC, SmallIcon("gear", currentItem->height()-2));
		currentItem->progressing = true;
	}
	else if(progress == -1){
		currentItem->setPixmap(ICON_LOC, SmallIcon("button_cancel", currentItem->height()-2));
	}
	else if(progress == 100){
		// Remove the job if requested.
		if(Prefs::removeCompletedJobs()){
			removeJob(currentItem, false);
			return;
		}
		currentItem->setPixmap(ICON_LOC, SmallIcon("button_ok", currentItem->height()));
	}
	if(currentJobCount > 0 && numberOfJobsNotFinished() == 0) 
		KNotifyClient::event("no jobs left");
}

/**
 * Remove job listed in item
 * @param item to remove. Note that it WILL be deleted and set to NULL.
 * @param kill kill the actual job/process. A bool here because this CAN cause
 *        a race condition when the encoder is 100%, but hasn't exited.
 * @param prompt the user if the job isn't finished
 * @return bool if remove was successfull or not.
 */
bool JobQueImp::removeJob(QueListViewItem *item, bool kill, bool prompt){
	if(!item)
		return false;

	if(item->percentDone < 100 && item->percentDone > -1 && (prompt && KMessageBox::questionYesNo(this, i18n("KAudioCreator has not finished %1. Remove anyway?").arg(item->text(HEADER_DESCRIPTION)), i18n("Unfinished Job in Queue"), KStdGuiItem::del(), i18n("Keep"))
		  == KMessageBox::No ))
		return false;

	// "Thread" safe
	if(!item)
		return false;

	if(kill)
		emit (removeJob(item->text(HEADER_JOB).toInt()));

	todoQue->takeItem(item);
	delete(item);
	item = NULL;

	// See if the Que needs to be updated...
	if(todoQue->childCount() == 0){
		queLabel->setText(i18n("No jobs are in the queue"));
		highestNumber = DEFAULT_HIGHEST_NUMBER;
		currentId = 0;
	}
	else
		queLabel->setText(i18n("Number of jobs in the queue: %1").arg(todoQue->childCount()));
	return true;
}

/**
 * Remove the currently selected Job
 */
void JobQueImp::removeSelectedJob(){
	QueListViewItem * currentItem = (QueListViewItem*)todoQue->firstChild();
	while(currentItem != NULL){
		if(currentItem->isSelected()){
			QueListViewItem *t = currentItem;
			currentItem = (QueListViewItem*)currentItem->nextSibling();
			removeJob(t);
		}
		else
			currentItem = (QueListViewItem*)currentItem->nextSibling();
	}
}

/**
 * Remove all of the jobs in the list.
 */
void JobQueImp::removeAllJobs(){
	// First determine if there are jobs not finished and prompt once here
	bool finished=true;
	QueListViewItem * item = (QueListViewItem*)todoQue->firstChild();
	while( item != NULL ){
		if(item && item->percentDone < 100 && item->percentDone > -1)
			finished = false;
		item = (QueListViewItem*)item->nextSibling();
	}
	if(!finished){
		if(KMessageBox::questionYesNo(this, i18n("KAudioCreator has not finished all of the jobs. Remove them anyway?"), i18n("Unfinished Job in Queue"), KStdGuiItem::del(), i18n("Keep"))
			== KMessageBox::No )
		return;
	}

	QueListViewItem * currentItem = (QueListViewItem*)todoQue->firstChild();
	while( currentItem != NULL ){
		QueListViewItem *next = (QueListViewItem*)currentItem->nextSibling();
		removeJob(currentItem, true, false);
		currentItem = next;
	}
}

/**
 * Remove any jobs that are in the list that are done.
 */
void JobQueImp::clearDoneJobs(){
	QueListViewItem * currentItem = (QueListViewItem*)todoQue->firstChild();
	while( currentItem != NULL ){
		QueListViewItem *itemToRemove = NULL;
		if( currentItem->percentDone == 100 ){
			itemToRemove = currentItem;
		}
		currentItem = (QueListViewItem*)currentItem->itemBelow();
		if(itemToRemove){
			emit (removeJob(itemToRemove->text(HEADER_JOB).toInt()));
			todoQue->takeItem(itemToRemove);
		}
	}
	if(todoQue->childCount() == 0){
		queLabel->setText(i18n("No jobs are in the queue"));
		highestNumber = DEFAULT_HIGHEST_NUMBER;
		currentId = 0;
	}
	else
		queLabel->setText(i18n("Number of jobs in the queue: %1").arg(todoQue->childCount()));
}

/**
 * Return the number of jobs in the que that don't have 100% or Error in the
 * Progress column
 * @return the number of jobs that are in the que that haven't been finished.
 */
int JobQueImp::numberOfJobsNotFinished(){
	int totalJobsToDo = 0;
	QueListViewItem * currentItem = (QueListViewItem*)todoQue->firstChild();
	while( currentItem != NULL ){
		if( currentItem->percentDone == 100 || currentItem->percentDone == -1 ){
		}
		else
			totalJobsToDo++;
		currentItem = (QueListViewItem*)currentItem->itemBelow();
	}
	return totalJobsToDo;
}

/**
 * The repaint function overloaded so that we can have a built in progressbar.
 */
void QueListViewItem::paintCell (QPainter * p,const QColorGroup &cg,int column,
			int width,int align){
	if(column != HEADER_PROGRESS){
		QListViewItem::paintCell(p,cg,column,width,align);
		return;
	}

	p->setPen(cg.base());
	p->drawRect(0,0,width,height());
	if(isSelected())
		p->fillRect(1,1,width-2,height()-2,cg.highlight());
	else
		p->fillRect(1,1,width-2,height()-2,cg.base());

	int percent = (int)(((double)(width-2)) * (percentDone/100));

	p->fillRect(1,1,percent,height()-2,cg.mid());

	// show the text
	p->setPen(cg.text());
	if(isSelected())
		p->setPen(cg.highlightedText());
	if(percentDone != -1)
	p->drawText(0,0,width-1,height()-1,AlignCenter,QString().setNum((int)percentDone) + "%");
	else
		p->drawText(0,0,width-1,height()-1,AlignCenter,i18n("Error"));
}

/**
 * Header for built in treelist item so we can have a progress bar in them.
 */
QueListViewItem::QueListViewItem(QListView *parent, const QString id, const QString p , const QString name, const QString d, const QString e) : QListViewItem(parent, id, p, name,d,e), percentDone(0), progressing(false) {
}

#include "jobqueimp.moc"

