/**
 * Copyright (C) 2003-2005 Benjamin C Meyer (ben at meyerhome dot net)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef ENCODER_H
#define ENCODER_H

#include <qobject.h>

#include <q3ptrlist.h>
#include <qmap.h>
#include "job.h"
#include <kprocess.h>

class EncoderPrefs;

class Encoder : public QObject {

Q_OBJECT

signals:
  void addJob(Job *job, const QString &name);
  void updateProgress(int id, int progress);
  void jobIsDone(Job *job, const QString &extension);
  void jobsChanged();

public:
  Encoder( QObject* parent = 0, const char* name = 0);
  ~Encoder();
  int activeJobCount();
  int pendingJobCount();

public slots:
  void removeJob(int id);
  void encodeWav(Job *job);
  void loadSettings();
	void loadEncoder( int encoder );

private slots:
  void receivedThreadOutput(KProcess *process, char *buffer, int buflen);
  void jobDone(KProcess *process);
  void tendToNewJobs();

private:
  Q3PtrList<Job> pendingJobs;
  Q3PtrList<KShellProcess> threads;
  QMap<KShellProcess*, Job*> jobs;

  EncoderPrefs *prefs;
  int reportCount;
};

#endif // ENCODER_H

