#include "queconfigimp.h"
#include <qlistview.h> 
#include <qpushbutton.h>
#include <qlabel.h>
#include "job.h"
#include <klocale.h>

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
  QListViewItem * newItem = new QListViewItem(todoQue, QString("%1%2").arg(buffer).arg(currentId), "0%", name);
  queLabel->setText(QString("Number of jobs in the queue: %1").arg(todoQue->childCount()));
}

/**
 * Locate the job and update the progress.
 * @param id the id of the job to update
 * @param progress the new progress of the job.
 */
void QueConfigImp::updateProgress(int id, int progress){
  QListViewItem * currentItem = todoQue->firstChild();
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
    currentItem = currentItem->nextSibling();
  }
  if( currentItem){
    if(progress != -1)
      currentItem->setText(HEADER_PROGRESS,QString("%1%").arg(progress));
    else
      currentItem->setText(HEADER_PROGRESS,i18n("Error"));
  }
}

/**
 * Remove the currently selected Job
 */
void QueConfigImp::removeSelectedJob(){
  QListViewItem * currentItem = todoQue->firstChild();
  while( currentItem != 0 ){
    if(currentItem->isSelected()){
      emit (removeJob(currentItem->text(HEADER_JOB).toInt()));
      QListViewItem *t = currentItem;
      currentItem = currentItem->nextSibling();
      todoQue->takeItem(t);
      delete(t);
    }
    else
      currentItem = currentItem->nextSibling();
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

  QListViewItem * currentItem = todoQue->firstChild();
  while( currentItem != 0 ){
    emit (removeJob(currentItem->text(HEADER_JOB).toInt()));
    todoQue->takeItem(currentItem);
    delete(currentItem);
    currentItem = todoQue->firstChild();
  }
}

/**
 * Remove any jobs that are in the list that are done.
 */
void QueConfigImp::clearDoneJobs(){
  QListViewItem * currentItem = todoQue->firstChild();
  while( currentItem != 0 ){
    QListViewItem *itemToRemove = NULL;
    if( currentItem->text(HEADER_PROGRESS) == "100%" || currentItem->text(HEADER_PROGRESS) == i18n("Error") ){
      itemToRemove = currentItem;
    }
    currentItem = currentItem->itemBelow();
    if(itemToRemove){
      emit (removeJob(itemToRemove->text(HEADER_JOB).toInt()));
      todoQue->takeItem(itemToRemove);
    }
  }
  if(todoQue->childCount() == 0)
    queLabel->setText(i18n("No jobs are in the queue"));
  else
    queLabel->setText(QString("Number of jobs in the queue: %1").arg(todoQue->childCount()));
}

/**
 * Return the number of jobs in the que that don't have 100% or Error marked down.
 * @param the number of jobs that are in the que that havn't been finished.
 */
int QueConfigImp::numberOfJobsNotFinished(){
  int totalJobsToDo = 0;
  QListViewItem * currentItem = todoQue->firstChild();
  while( currentItem != 0 ){
    if( currentItem->text(HEADER_PROGRESS) != "100%" || currentItem->text(HEADER_PROGRESS) != i18n("Error") ){
    }
    totalJobsToDo++;
    currentItem = currentItem->itemBelow();
  }
  return totalJobsToDo;
}

#include "queconfigimp.moc"

// queconfigimp.cpp

