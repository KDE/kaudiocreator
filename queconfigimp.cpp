#include "queconfigimp.h"
#include "job.h"
#include <qlistview.h> 
#include <qpushbutton.h>
#include <qlabel.h>
#include <qpainter.h>
#include <klocale.h>

#define HEADER_JOB 0
#define HEADER_PROGRESS 1
#define HEADER_DESCRIPTION 2

#define DEFAULT_HIGHEST_NUMBER 9

/**
 * Constructor, set up signals.
 * @param parent - parent widget
 * @param name - widget name
 */
QueConfigImp::QueConfigImp( QWidget* parent, const char* name):QueConfig(parent,name),highestNumber(DEFAULT_HIGHEST_NUMBER), currentId(0){
  connect(removeSelected,SIGNAL(clicked()), this, SLOT( removeSelectedJob()));
  connect(removeAll, SIGNAL(clicked()), this, SLOT(removeAllJobs()));
  connect(removeDoneJobs, SIGNAL(clicked()), this, SLOT(clearDoneJobs()));
}

/***
 * Return a buffer of "000" so that new, updated jobs strings will be able to sort via the columns.
 * Based upon a highest number that is kept. 
 * @param number the number to fill out.
 */ 
QString QueConfigImp::getStringFromNumber(int number){
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
void QueConfigImp::addJob(Job*job, QString name ){
  job->id = ++currentId;
  QueListViewItem * newItem = new QueListViewItem(todoQue, QString("%1%2").arg(getStringFromNumber(currentId)).arg(currentId), "0", name);
  queLabel->setText(i18n("Number of jobs in the queue: %1").arg(todoQue->childCount()));
}

/**
 * Locate the job and update the progress.
 * @param id the id of the job to update
 * @param progress the new progress of the job.
 */
void QueConfigImp::updateProgress(int id, int progress){
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
    }
  }
  else{
    qDebug("An update was recieved about a job, but the job couldn't be found: %d", id);
  }
}

/**
 * Remove the currently selected Job
 */
void QueConfigImp::removeSelectedJob(){
  QueListViewItem * currentItem = (QueListViewItem*)todoQue->firstChild();
  while( currentItem != 0 ){
    if(currentItem->isSelected()){
      emit (removeJob(currentItem->text(HEADER_JOB).toInt()));
      QueListViewItem *t = currentItem;
      currentItem = (QueListViewItem*)currentItem->nextSibling();
      todoQue->takeItem(t);
      delete(t);
    }
    else
      currentItem = (QueListViewItem*)currentItem->nextSibling();
  }

  if(todoQue->childCount() == 0){
    queLabel->setText(i18n("No jobs are in the queue"));
    highestNumber = DEFAULT_HIGHEST_NUMBER;
  }
  else
    queLabel->setText(i18n("Number of jobs in the queue: %1").arg(todoQue->childCount()));
}

/**
 * Remove all of the jobs in the list.
 */
void QueConfigImp::removeAllJobs(){
  queLabel->setText(i18n("No jobs are in the queue"));

  QueListViewItem * currentItem = (QueListViewItem*)todoQue->firstChild();
  while( currentItem != 0 ){
    emit (removeJob(currentItem->text(HEADER_JOB).toInt()));
    todoQue->takeItem(currentItem);
    delete(currentItem);
    currentItem = (QueListViewItem*)todoQue->firstChild();
  }
  highestNumber = DEFAULT_HIGHEST_NUMBER;
}

/**
 * Remove any jobs that are in the list that are done.
 */
void QueConfigImp::clearDoneJobs(){
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
  }
  else
    queLabel->setText(i18n("Number of jobs in the queue: %1").arg(todoQue->childCount()));
}

/**
 * Return the number of jobs in the que that don't have 100% or Error in the
 * Progress column
 * @return the number of jobs that are in the que that havn't been finished.
 */
int QueConfigImp::numberOfJobsNotFinished(){
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

#include "queconfigimp.moc"

// queconfigimp.cpp

