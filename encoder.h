/**
 * This file is part of the KAudioCreator package
 * Copyright (C) 2003 Benjamin C Meyer (ben+kaudiocreator at meyerhome dot net)
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
 * the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

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
  bool relativeURL(const QString &path1, const QString &path2,
                   QString &relativePath ) const;
  
  QPtrList<Job> pendingJobs;
  QPtrList<KShellProcess> threads;
  QMap<KShellProcess*, Job*> jobs;
  void appendToPlaylist(Job* job);

  // Class KConfig Settings
  QString encoderCommandLine;
  QString encoderExtension;
  int encoderPercentLength;
  uint numberOfCpus;
  bool setNiceLevel;
  int niceLevel;
  QString fileFormat;
  bool createPlaylist;
  QString playlistFileFormat;
  bool useRelitivePath;

  QString replaceInput;
  QString replaceOutput;
};

#endif

