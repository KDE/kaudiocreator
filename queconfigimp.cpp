#include "queconfigimp.h"
#include <qlistview.h> 
#include <qpushbutton.h>
#include <qlabel.h>
#include "job.h"
#include <klocale.h>


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
  QListViewItem * newItem = new QListViewItem(todoQue, QString("%1%2").arg(buffer).arg(currentId), name, "0%");
  queLabel->setText(QString("Number of jobs in the que: %1").arg(todoQue->childCount()));
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
    if(currentItem->text(0) == buffer)
      break;
    currentItem = currentItem->nextSibling();
  }
  if( currentItem){
    if(progress != -1)
      currentItem->setText(2,QString("%1%").arg(progress));
    else
      currentItem->setText(2,i18n("Error"));
  }
}

/**
 * Remove the currently selected Job
 */
void QueConfigImp::removeSelectedJob(){
  QListViewItem * currentItem = todoQue->firstChild();
  while( currentItem != 0 ){
    if(currentItem->isSelected()){
      emit (removeJob(currentItem->text(0).toInt()));
      QListViewItem *t = currentItem;
      currentItem = currentItem->nextSibling();
      todoQue->takeItem(t);
      delete(t);
    }
    else
      currentItem = currentItem->nextSibling();
  }

  if(todoQue->childCount() == 0)
    queLabel->setText(i18n("No jobs are in the que"));
  else
    queLabel->setText(QString("Number of jobs in the que: %1").arg(todoQue->childCount()));
}

/**
 * Remove all of the jobs in the list.
 */
void QueConfigImp::removeAllJobs(){
  queLabel->setText(i18n("No jobs are in the que"));

  QListViewItem * currentItem = todoQue->firstChild();
  while( currentItem != 0 ){
    emit (removeJob(currentItem->text(0).toInt()));
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
    if( currentItem->text(2) == "100%" || currentItem->text(2) == i18n("Error") ){
      itemToRemove = currentItem;
    }
    currentItem = currentItem->itemBelow();
    if(itemToRemove){
      emit (removeJob(itemToRemove->text(0).toInt()));
      todoQue->takeItem(itemToRemove);
    }
  }
  if(todoQue->childCount() == 0)
    queLabel->setText(i18n("No jobs are in the que"));
  else
    queLabel->setText(QString("Number of jobs in the que: %1").arg(todoQue->childCount()));

}

#include "queconfigimp.moc"

// queconfigimp.cpp

