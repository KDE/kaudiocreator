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
	void addJob( Job *job, const QString &name );
	void updateProgress( int id, int progress );
	void encodeWav( Job *job );
	void eject( const QString &device );
	void jobsChanged();

public:
	Ripper( QObject* parent = 0, const char* name = 0 );
	~Ripper();
	int activeJobCount();
	int pendingJobCount();

public slots:
	void loadSettings();
	void ripTrack( Job * );
	void removeJob( int id );

private slots:
	void copyJobResult( KIO::Job *job );
	void updateProgress( KIO::Job *job, unsigned long percent );
	void tendToNewJobs();
	void ejectNow();

private:
	QString deviceToEject;
	// Jobs that we are currently doing.
	QMap<KIO::Job*, Job*> jobs;
	// Jobs that we want to do , but haven't done yet
	QPtrList<Job> pendingJobs;
};

#endif // RIPPER_H

