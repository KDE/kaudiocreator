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
#include <qpushbutton.h>
#include <qlabel.h>
#include <qpainter.h>
#include <kconfig.h>
#include <kglobal.h>
#include <kdebug.h>

#define HEADER_JOB 0
#define HEADER_PROGRESS 1
#define HEADER_DESCRIPTION 2

#define DEFAULT_HIGHEST_NUMBER 9

/**
 * Constructor, set up signals.
 * @param parent - parent widget
 * @param name - widget name
 */
JobQueImp::JobQueImp( QWidget* parent, const char* name):JobQue(parent,name),highestNumber(DEFAULT_HIGHEST_NUMBER), currentId(0){
  connect(removeSelected,SIGNAL(clicked()), this, SLOT( removeSelectedJob()));
  connect(removeAll, SIGNAL(clicked()), this, SLOT(removeAllJobs()));
  connect(removeDoneJobs, SIGNAL(clicked()), this, SLOT(clearDoneJobs()));
  loadSettings();
}

/**
 * Loads the settings
 */
void JobQueImp::loadSettings(){
  KConfig &config = *KGlobal::config();
  config.setGroup("General");
  removeCompletedJobs = config.readBoolEntry("removeCompletedJobs", false);
}

/***
 * Return a buffer of "000" so that new, updated jobs strings will be able to sort via the columns.
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
      while( currentItem != 0 ){
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
void JobQueImp::addJob(Job*job, QString name ){
  job->id = ++currentId;
  (void)new QueListViewItem(todoQue, QString("%1%2").arg(getStringFromNumber(currentId)).arg(currentId), "0", name);
  queLabel->setText(i18n("Number of jobs in the queue: %1").arg(todoQue->childCount()));
}

/**
 * Locate the job and update the progress.
 * @param id the id of the job to update
 * @param progress the new progress of the job.
 */
void JobQueImp::updateProgress(int id, int progress){
  QueListViewItem * currentItem = (QueListViewItem*)todoQue->firstChild();
  QString buffer = getStringFromNumber(id);
  buffer += QString("%1").arg(id);
  while( currentItem != 0 ){
    if(currentItem->text(HEADER_JOB) == buffer)
      break;
    currentItem = (QueListViewItem*)currentItem->nextSibling();
  }
  
  if( currentItem){
    if(currentItem->percentDone != progress){
      currentItem->percentDone = progress;
      currentItem->repaint();
      if(removeCompletedJobs && progress == 100)
	removeJob(currentItem);      
    }
  }
  else{
    kdDebug() << "An update was received about a job, but the job couldn't be found: " << id << endl;
  }
}

/**
 * Remove job listed in item
 * @param item to remove.  Note that it WILL be deleted and set to NULL.
 */ 
void JobQueImp::removeJob(QueListViewItem *item){
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
  QueListViewItem * currentItem = NULL;
  while( (currentItem = (QueListViewItem*)todoQue->firstChild()) != NULL ){
    removeJob(currentItem);
  }
}

/**
 * Remove any jobs that are in the list that are done.
 */
void JobQueImp::clearDoneJobs(){
  QueListViewItem * currentItem = (QueListViewItem*)todoQue->firstChild();
  while( currentItem != 0 ){
    QueListViewItem *itemToRemove = NULL;
    if( currentItem->percentDone == 100 || currentItem->percentDone == -1 ){
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
  while( currentItem != 0 ){
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
  if(this->isSelected())
    p->fillRect(1,1,width-2,height()-2,cg.highlight());
  else
    p->fillRect(1,1,width-2,height()-2,cg.base());
  
  int percent = (int)(((double)(width-2)) * (percentDone/100));

  p->fillRect(1,1,percent,height()-2,cg.mid());

  // show the text
  p->setPen(cg.text());
  if(this->isSelected())
    p->setPen(cg.highlightedText());
  if(percentDone != -1)
  p->drawText(0,0,width-1,height()-1,AlignCenter,QString().setNum((int)percentDone) + "%");
  else
    p->drawText(0,0,width-1,height()-1,AlignCenter,i18n("Error"));
}

/**
 * Header for built in treelist item so we can have a progress bar in them.
 */
QueListViewItem::QueListViewItem(QListView *parent, QString id, QString p , QString name) : QListViewItem(parent, id, p, name){
  percentDone = 0;
}

#include "jobqueimp.moc"

