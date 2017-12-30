/**
 * Copyright (C) 2003-2005 Benjamin C Meyer (ben at meyerhome dot net)
 * Copyright (C) 2009 Gerd Fleischer (gerdfleischer at web dot de)
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
#include <QMap>

#include <KProcess>

#include "job.h"

class EncoderPrefs;

class EncodeProcess : public KProcess
{
	Q_OBJECT

	public:
		EncodeProcess(QObject *parent = nullptr)
			: KProcess(parent) {
			connect(this, SIGNAL(readyReadStandardOutput()),
				this, SLOT(newStandardOutput()));
			connect(this, SIGNAL(finished(int, QProcess::ExitStatus)),
				this, SLOT(done(int, QProcess::ExitStatus)));
		}

	private slots:
		void newStandardOutput() {
			emit newEncodeOutput(this);
		}

		void done(int exitCode, QProcess::ExitStatus exitStatus) {
			emit encodingFinished(this, exitCode, exitStatus);
		}

	signals:
		void newEncodeOutput(EncodeProcess *);
		void encodingFinished(KProcess *, int, QProcess::ExitStatus);
};

class Encoder : public QObject {

Q_OBJECT

signals:
  void addJob(Job *job, const QString &name);
  void updateProgress(int id, int progress);
  void jobsChanged();

public:
  Encoder( QObject* parent = nullptr );
  ~Encoder();
  int activeJobCount();
  int pendingJobCount();

public slots:
  void removeJob(int id);
  void encodeWav(Job *job);
  void loadSettings();
  EncoderPrefs* loadEncoder( QString encoder );

private slots:
  void receivedThreadOutput(EncodeProcess *process);
  void jobDone(KProcess *process);
  void tendToNewJobs();

private:
  QList<Job *> pendingJobs;
  QList<KProcess *> threads;
  QMap<KProcess *, Job *> jobs;

  int reportCount;
};

#endif // ENCODER_H

