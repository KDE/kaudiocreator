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
#include <qptrlist.h>
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
  void playlistWizard();
  void encoderWizard();

private:
  QList<Job> pendingJobs;
  QList<KShellProcess> threads;
  QMap<KShellProcess*, Job*> jobs;
  void appendToPlaylist(Job* job);
  int encodersPercentStringLength;
  int oldEncoderSelection;

  bool save;
  QMap<int, QString> encoderName;
  QMap<int, QString> encoderArgs;
  QMap<int, QString> encoderExtension;
  QMap<int, int> encoderpercentLength;
};

#endif

// encoderconfigimp.h

