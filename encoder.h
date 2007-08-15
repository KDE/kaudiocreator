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

#include <QObject>

#include <q3ptrlist.h>
#include <QMap>
#include "job.h"
#include <k3process.h>

class EncoderPrefs;

class Encoder : public QObject {

Q_OBJECT

signals:
  void addJob(Job *job, const QString &name);
  void updateProgress(int id, int progress);
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
  void receivedThreadOutput(K3Process *process, char *buffer, int buflen);
  void jobDone(K3Process *process);
  void tendToNewJobs();

private:
  Q3PtrList<Job> pendingJobs;
  Q3PtrList<K3ShellProcess> threads;
  QMap<K3ShellProcess*, Job*> jobs;

  EncoderPrefs *prefs;
  int reportCount;
};

#endif // ENCODER_H

