#include "queconfigimp.h"
#include <qlistview.h> 
#include <qpushbutton.h>
#include <qlabel.h>
#include "job.h"
#include <klocale.h>

#include <qpainter.h>

#define HEADER_JOB 0
#define HEADER_PROGRESS 1
#define HEADER_DESCRIPTION 2


/**
 * Constructor, set up signals.
 */
QueConfigImp::QueConfigImp( QWidget* parent, const char* name):QueConfig(parent,name),currentId(0){
  connect(removeSelected,SIGNAL(clicked()), this, SLOT( removeSelectedJob()));
  connect(removeAll, SIGNAL(clicked()), this, SLOT(removeAllJobs()));
  connect(removeDoneJobs, SIGNAL(clicked()), this, SLOT(clearDoneJobs()));
}

/** 
 * Add a new job to the que
 * @param id the id of the job.
 * @param name the name of the job.
 */
void QueConfigImp::addJob(Job*job, QString name ){
  job->id = ++currentId;
  QString buffer = "";
  if( job->id < 1000 )
    buffer += "0";
  if( job->id < 100 )
    buffer += "0";
  if( job->id < 10 )
    buffer += "0";
  QueListViewItem * newItem = new QueListViewItem(todoQue, QString("%1%2").arg(buffer).arg(currentId), "0", name);
  queLabel->setText(i18n("Number of jobs in the queue: %1").arg(todoQue->childCount()));
}

/**
 * Locate the job and update the progress.
 * @param id the id of the job to update
 * @param progress the new progress of the job.
 */
void QueConfigImp::updateProgress(int id, int progress){
  QueListViewItem * currentItem = (QueListViewItem*)todoQue->firstChild();
  QString buffer = "";
  if( id < 1000 )
    buffer += "0";
  if( id < 100 )
    buffer += "0";
  if( id < 10 )
    buffer += "0";
  buffer += QString("%1").arg(id);
  while( currentItem != 0 ){
    if(currentItem->text(HEADER_JOB) == buffer)
      break;
    currentItem = (QueListViewItem*)currentItem->nextSibling();
  }
  if( currentItem){
    currentItem->percentDone = progress;
    currentItem->repaint();
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

  if(todoQue->childCount() == 0)
    queLabel->setText(i18n("No jobs are in the queue"));
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
  if(todoQue->childCount() == 0)
    queLabel->setText(i18n("No jobs are in the queue"));
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
  if(!this->isSelected())
    p->fillRect(1,1,width-2,height()-2,cg.base());
  else
    p->fillRect(1,1,width-2,height()-2,cg.highlight());
  int percent = (int)((double)(width-2)/(double)100* (double)percentDone);

  p->fillRect(1,1,percent,height()-2,cg.mid());

  // show the text
  p->setPen(cg.text());
  if(this->isSelected())
    p->setPen(cg.highlightedText());
  if(percentDone != -1)
  p->drawText(0,0,width-1,height()-1,AlignCenter,QString().setNum(percentDone) + "%");
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

