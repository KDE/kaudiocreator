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

class Job;

class QueConfigImp : public QueConfig  {

Q_OBJECT

signals:
  void removeJob( int idNumber );

public:
  QueConfigImp( QWidget* parent = 0, const char* name = 0);

public slots:
  void updateProgress(int id, int progress);
  void addJob(Job* job, QString name);
  void clearDoneJobs();

private slots:
  void removeSelectedJob();
  void removeAllJobs();

private:
  int currentId;
};

#endif

// queconfigimp.h

