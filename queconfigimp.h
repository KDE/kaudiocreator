/***************************************************************************
                               queconfigimp.h
                             -------------------
    begin                : Friday Feb 9 2002
    copyright            : (C) 2001 by Benjamin Meyer
    email                : ben-dev@meyerhome.net
 ***************************************************************************/

#ifndef QUECONFIGIMP_H
#define QUECONFIGIMP_H

#include "queconfig.h"
#include <qstring.h>
#include <qlistview.h>

class Job;
class QPainter;
class QColorGroup;
class QListView;

class QueConfigImp : public QueConfig  {

Q_OBJECT

signals:
  void removeJob( int idNumber );

public:
  QueConfigImp( QWidget* parent = 0, const char* name = 0);
  int numberOfJobsNotFinished();

public slots:
  void updateProgress(int id, int progress);
  void addJob(Job* job, QString name);
  void clearDoneJobs();

private slots:
  void removeSelectedJob();
  void removeAllJobs();

private:
  QString getStringFromNumber(int number);
  int highestNumber;

  int currentId;

};

/**
 * Helper class to allow for progress bars in list view items.
 */
class QueListViewItem : public QListViewItem {

public:
  QueListViewItem (QListView * p = NULL, QString a=0, QString b=0, QString=0);
  virtual void paintCell (QPainter * p,const QColorGroup &cg,int column,
	    int width,int align);
  int percentDone;
};

#endif

// queconfigimp.h

