/***************************************************************************
                                ripconfig.h
                             -------------------
    begin                : Friday Feb 9 2002
    copyright            : (C) 2001 by Benjamin Meyer
    email                : ben-dev@meyerhome.net
 ***************************************************************************/

#ifndef RIPCONFIGIMP_H
#define RIPCONFIGIMP_H

#include "ripconfig.h"
#include "job.h"
#include <kio/jobclasses.h>
#include <qmap.h>
#include <qptrlist.h>

class KURL;
class Job;

class RipConfigImp : public RipConfig  {

Q_OBJECT

signals:
  void addJob(Job *job, QString name);
  void updateProgress(int id, int progress);
  void encodeWav(Job *job);

public:
  RipConfigImp( QWidget* parent = 0, const char* name = 0);
  ~RipConfigImp();

public slots:
  void ripTrack(Job *);
  void removeJob(int id);

private slots:
  void copyJobResult(KIO::Job *job);
  void updateProgress( KIO::Job *job, unsigned long percent);
  void tendToNewJobs();
  void eject();

private:
  // Jobs that we are currently doing.
  QMap<KIO::Job*, Job*> jobs;
  // Jobs that we want to do , but havn't done yet
  QList<Job> pendingJobs;

};

#endif

// ripconfig.h

