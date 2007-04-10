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

#include "ripper.h"
#include "prefs.h"

#include <QFile>
#include <QTimer>
#include <ktemporaryfile.h>
#include <kmessagebox.h>
#include <knotification.h>
#include <kstandarddirs.h>
#include <kio/scheduler.h>
#include <kio/jobuidelegate.h>
/**
 * Constructor, load settings.
 */
Ripper::Ripper( QObject* parent) : QObject(parent) {
	loadSettings();
}

/**
 * Loads the settings
 */
void Ripper::loadSettings(){
	for(uint i=0; i<(uint)Prefs::maxWavFiles(); i++)
		tendToNewJobs();
}

/**
 * Deconstructor, remove pending jobs, remove current jobs, save settings.
 */
Ripper::~Ripper(){
	pendingJobs.clear();
	QMap<KIO::Job*, Job*>::Iterator it;
	for( it = jobs.begin(); it != jobs.end(); ++it ){
		 KIO::Job* ioJob = it.key();
		Job *job = it.data();
		delete job;

		if(ioJob){
			KIO::FileCopyJob *copyJob = static_cast<KIO::FileCopyJob*> (ioJob);
			disconnect(copyJob, SIGNAL(result(KJob*)), this, SLOT(copyJobResult(KJob*)));
			disconnect(copyJob, SIGNAL(percent ( KJob *, unsigned long)), this, SLOT(updateProgress ( KJob *, unsigned long)));
			QString fileDestination = (copyJob->destUrl()).path();
			copyJob->kill();
			QFile file( fileDestination );
			file.remove();
		}
	}
	jobs.clear();
}

/**
 * @return The number of active jobs
 */
int Ripper::activeJobCount() {
	return jobs.count();
}

/**
 * @return The number of pending jobs
 */
int Ripper::pendingJobCount() {
	return pendingJobs.count();
}

/**
 * Cancel and remove the job with the matching id.
 * Remove it from the local collection of jobs, delete the temp file if
 * there is one.
 * @param id the id number of the job to remove.
 */
void Ripper::removeJob(int id){
	QMap<KIO::Job*, Job*>::Iterator it;
	for( it = jobs.begin(); it != jobs.end(); ++it ){
		if(it.data()->id == id){
			jobs.remove(it.key());
			KIO::FileCopyJob *copyJob = dynamic_cast<KIO::FileCopyJob*> (it.key());
			if(copyJob){
				QString fileDestination = (copyJob->destUrl()).path();
				copyJob->kill();
				// This here is such a hack, shouldn't kill() do this, or why isn't there a stop()?
				// TODO add to copyJob a stop() function.
				QFile file( fileDestination );
				if(file.exists())
					file.remove();
				else {
					QFile f( fileDestination+".part" );
					f.remove();
				}
			}
			break;
		}
	}
	Job *job = pendingJobs.first();
	while(job){
		if(job->id == id)
			break;
		job = pendingJobs.next();
	}
	if(job){
		pendingJobs.remove(job);
		delete job;
	}
	//qDebug(QString("Done removing Job:%1").arg(id).latin1());
	tendToNewJobs();
}

/**
 * Begin to rip the track specified in job.
 * @param job the new job that this module should take over.
 * @param job the new job that we need to handle.
 */
void Ripper::ripTrack(Job *job){
	if(!job)
		return;
	emit(addJob(job, i18n("Ripping: %1 - %2", job->track_artist, job->track_title)));
	pendingJobs.append(job);
	tendToNewJobs();
}

/**
 * See if there are are new jobs to attend too.  If we are all loaded up
 * then just loop.
 */
void Ripper::tendToNewJobs(){
	if(pendingJobs.count() == 0){
		emit jobsChanged();
		return;
	}

	// If we are currently ripping the max try again in a little bit.
	if(jobs.count() >= (uint)Prefs::maxWavFiles()){
		emit jobsChanged();
		return;
	}

	Job *job = pendingJobs.first();
	if(!job)
		return;
	pendingJobs.remove(job);

	QMap<QString, QString> map;
	QString defaultTempDir;
	if(Prefs::enableTempDir())
		defaultTempDir = Prefs::tempDir();
	else
		defaultTempDir = KStandardDirs::locateLocal("tmp", "");
	// For cases like "/tmp" where there is a missing /
	defaultTempDir = KUrl::fromPathOrUrl(defaultTempDir).path(KUrl::AddTrailingSlash);
	kDebug() << "defaultTempDir: " << defaultTempDir << endl;
	KTemporaryFile tmp;
	tmp.setPrefix(defaultTempDir);
	tmp.setSuffix(".wav");
	tmp.open();

	QString wavFile;
	if(job->track < 10)
		wavFile = QString("audiocd:/Wav/Track 0%1.wav").arg(job->track);
	else
		wavFile = QString("audiocd:/Wav/Track %1.wav").arg(job->track);

	KUrl source = KUrl::fromPathOrUrl(wavFile);
	kDebug() << "source: " << source << endl;
	if (!job->device.isEmpty())
		source.addQueryItem("device", job->device);
	source.addQueryItem("fileNameTemplate", "Track %{number}");
	KUrl dest = KUrl::fromPathOrUrl(tmp.fileName());
	kDebug() << "dest: " << dest << endl;

	KIO::FileCopyJob *copyJob = new KIO::FileCopyJob(source, dest, 0644, false, true, false);
	jobs.insert(copyJob, job);
	connect(copyJob, SIGNAL(result(KJob*)), this, SLOT(copyJobResult(KJob*)));
	connect(copyJob, SIGNAL(percent ( KJob *, unsigned long)), this, SLOT(updateProgress ( KJob *, unsigned long)));

	emit jobsChanged();
}

/**
 * Copies the data from the KIO Job.  Uses this data to fill in the
 * information dialog.
 * @param copyjob the IO job to copy from
 */
void Ripper::copyJobResult(KJob *copyjob){
	if(!copyjob)
		return;
	KIO::FileCopyJob *copyJob = static_cast<KIO::FileCopyJob*> (copyjob);
	KNotification::event("track ripped");

	if(jobs.find(static_cast<KIO::Job*>(copyjob)) == jobs.end())
		return;
	Job *newJob = jobs[static_cast<KIO::Job*>(copyjob)];
	jobs.remove(static_cast<KIO::Job*>(copyjob));

	if(Prefs::beepAfterRip())
		KNotification::beep();

	if ( copyJob->error() == 0 ){
		emit updateProgress(newJob->id, 100);
		newJob->location = copyJob->destUrl().path();
		emit( encodeWav(newJob));
	}
	else{
		copyJob->ui()->setWindow(0);
		copyJob->ui()->showErrorMessage();
		QFile file( (copyJob->destUrl()).path());
		file.remove();
		emit updateProgress(newJob->id, -1);
		delete newJob;
		newJob = 0;
	}

	if(newJob && newJob->lastSongInAlbum){
		if(Prefs::autoEjectAfterRip()){
			// Don't eject device if a pending job has that device
			Job *job = pendingJobs.first();
			while( job ){
				if( job->device == newJob->device )
					break;
				job = pendingJobs.next();
			}
			if( !job ){
				deviceToEject = newJob->device;
				QTimer::singleShot( Prefs::autoEjectDelay()*1000 + 500, this, SLOT(ejectNow()));
			}
		}
		KNotification::event("cd ripped");
	}
	tendToNewJobs();
}

void Ripper::ejectNow(){
	emit eject(deviceToEject);
}

/**
 * Update the progress of the file copy.
 * @param job the current ioslave job in progress
 * @param percent the current percent that the ioslave has done.
 */
void Ripper::updateProgress( KJob *job, unsigned long percent){
	if(job){
		Job *ripJob = (jobs[static_cast<KIO::Job*>(job)]);
		if(ripJob)
			emit updateProgress(ripJob->id, percent);
	}
}

#include "ripper.moc"

