/***************************************************************************
                             encoderconfigimp.h
                             -------------------
    begin                : Friday Feb 9 2002
    copyright            : (C) 2001 by Benjamin Meyer
    email                : ben-dev@meyerhome.net
 ***************************************************************************/

#ifndef ENCODERCONFIGIMP_H
#define ENCODERCONFIGIMP_H

#include <qobject.h>

#include <qptrlist.h>
#include <qmap.h>
#include "job.h"
#include <kprocess.h>

class EncoderConfigImp : public QObject {

Q_OBJECT

signals:
  void addJob(Job *job, QString name);
  void updateProgress(int id, int progress);

public:
  EncoderConfigImp( QObject* parent = 0, const char* name = 0);
  ~EncoderConfigImp();

public slots:
  void removeJob(int id);
  void encodeWav(Job *job);
  void loadSettings();

private slots:
  void receivedThreadOutput(KProcess *process, char *buffer, int buflen);
  void jobDone(KProcess *process);
  void tendToNewJobs();

private:
  QPtrList<Job> pendingJobs;
  QPtrList<KShellProcess> threads;
  QMap<KShellProcess*, Job*> jobs;
  void appendToPlaylist(Job* job);

  // Class KConfig Settings
  QString encoderCommandLine;
  QString encoderExtension;
  int encoderPercentLenght;
  bool saveWav;
  uint numberOfCpus;
  QString fileFormat;
  bool createPlaylist;
  QString playlistFileFormat;
  bool useRelitivePath;
};

#endif

