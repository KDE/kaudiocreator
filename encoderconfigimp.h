/***************************************************************************
                             encoderconfigimp.h
                             -------------------
    begin                : Friday Feb 9 2002
    copyright            : (C) 2001 by Benjamin Meyer
    email                : ben-dev@meyerhome.net
 ***************************************************************************/

#ifndef ENCODERCONFIGIMP_H
#define ENCODERCONFIGIMP_H

#include <qwidget.h>
#include <qlist.h>
#include <qmap.h>
#include <qthread.h>
#include <qobject.h>
#include "encoderconfig.h"
#include "job.h"
#include <kprocess.h>

class EncoderConfigImp : public EncoderConfig {

Q_OBJECT

signals:
  void addJob(Job *job, QString name);
  void updateProgress(int id, int progress);

public:
  EncoderConfigImp( QWidget* parent = 0, const char* name = 0);
  ~EncoderConfigImp();

public slots:
  void removeJob(int id);
  void encodeWav(Job *job);

private slots:
  void receivedThreadOutput(KProcess *process, char *buffer, int buflen);
  void jobDone(KProcess *process);
  void tendToNewJobs();
  void loadEncoderConfig(int index);

private:
  QList<Job> pendingJobs;
  QList<KProcess> threads;
  QMap<KProcess*, Job*> jobs;
  void appendToPlaylist(Job* job);

};

#endif

// encoderconfigimp.h

