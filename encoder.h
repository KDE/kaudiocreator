/***************************************************************************
                             encoder.h
                             -------------------
    begin                : Friday Feb 9 2002
    copyright            : (C) 2001 by Benjamin Meyer
    email                : ben-dev@meyerhome.net
 ***************************************************************************/

#ifndef ENCODER_H
#define ENCODER_H

#include <qobject.h>

#include <qptrlist.h>
#include <qmap.h>
#include "job.h"
#include <kprocess.h>

class Encoder : public QObject {

Q_OBJECT

signals:
  void addJob(Job *job, QString name);
  void updateProgress(int id, int progress);

public:
  Encoder( QObject* parent = 0, const char* name = 0);
  ~Encoder();

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
  int encoderPercentLength;
  uint numberOfCpus;
  QString fileFormat;
  bool createPlaylist;
  QString playlistFileFormat;
  bool useRelitivePath;

  QString replaceInput;
  QString replaceOutput;
};

#endif
