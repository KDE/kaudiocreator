/***************************************************************************
                               jobqueimp.h
                             -------------------
    begin                : Friday Feb 9 2002
    copyright            : (C) 2001 by Benjamin Meyer
    email                : ben-dev@meyerhome.net
 ***************************************************************************/

#ifndef JOBQUEIMP_H
#define JOBQUEIMP_H

#include "jobque.h"
#include <qstring.h>
#include <qlistview.h>

class Job;
class QPainter;
class QColorGroup;
class QListView;

/**
 * Helper class to allow for progress bars in list view items.
 */
class QueListViewItem : public QListViewItem {

public:
  QueListViewItem (QListView * p = NULL, QString a=0, QString b=0, QString=0);
  virtual void paintCell (QPainter * p,const QColorGroup &cg,int column,
	    int width,int align);
  double percentDone;
};


class JobQueImp : public JobQue  {

Q_OBJECT

signals:
  void removeJob( int idNumber );

public:
  JobQueImp( QWidget* parent = 0, const char* name = 0);
  int numberOfJobsNotFinished();

public slots:
  void loadSettings();
  void updateProgress(int id, int progress);
  void addJob(Job* job, QString name);
  
  // Toolbar Button
  void clearDoneJobs();
  
private slots:
  void removeSelectedJob();
  void removeAllJobs();

private:
  void removeJob(QueListViewItem *item);
  QString getStringFromNumber(int number);
  int highestNumber;

  int currentId;
  bool removeCompletedJobs;
};

#endif

