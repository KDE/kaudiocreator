/***************************************************************************
                                ripconfig.h
                             -------------------
    begin                : Friday Feb 9 2002
    copyright            : (C) 2001 by Benjamin Meyer
    email                : ben-dev@meyerhome.net
 ***************************************************************************/

#ifndef RIPPER_H
#define RIPPER_H

#include "job.h"
#include <kio/jobclasses.h>
#include <qmap.h>
#include <qptrlist.h>

class Job;

class Ripper : public QObject {

Q_OBJECT

signals:
  void addJob(Job *job, QString name);
  void updateProgress(int id, int progress);
  void encodeWav(Job *job);
  void eject();

public:
  Ripper( QObject* parent = 0, const char* name = 0);
  ~Ripper();
  
public slots:
  void loadSettings();
  void ripTrack(Job *);
  void removeJob(int id);

private slots:
  void copyJobResult(KIO::Job *job);
  void updateProgress( KIO::Job *job, unsigned long percent);
  void tendToNewJobs();

private:
  // Jobs that we are currently doing.
  QMap<KIO::Job*, Job*> jobs;
  // Jobs that we want to do , but haven't done yet
  QPtrList<Job> pendingJobs;

  int maxWavFiles;
  bool beepAfterRip;
  bool autoEjectAfterRip;
  int autoEjectDelay;

};

#endif

