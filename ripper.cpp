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
#include "defs.h"

#include <QFile>
#include <QTimer>

#include <krandom.h>
#include <kmessagebox.h>
#include <knotification.h>
#include <kstandarddirs.h>
#include <kio/scheduler.h>
#include <kio/jobuidelegate.h>
/**
 * Constructor, load settings.
 */
Ripper::Ripper( QObject *parent) : QObject(parent)
{
    tendToNewJobs();
}

/**
 * Deconstructor, remove pending jobs, remove current jobs, save settings.
 */
Ripper::~Ripper()
{
	qDeleteAll(pendingJobs);
	pendingJobs.clear();

	QMap<KIO::Job*, Job*>::Iterator it;
	for( it = jobs.begin(); it != jobs.end(); ++it ){
		 KIO::Job* ioJob = it.key();
		Job *job = it.value();
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
		if(it.value()->id == id){
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
			jobs.remove(it.key());
			break;
		}
	}
	Job *job = 0;
	foreach(Job *j, pendingJobs)
	{
		if(j->id == id)
		{
			job = j;
			break;
		}
	}
	if(job){
		pendingJobs.removeAll(job);
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
 * See if there are are new jobs to attend too.  If device is busy
 * then just loop.
 */
void Ripper::tendToNewJobs()
{
    if (pendingJobs.count() == 0) {
        emit jobsChanged();
        return;
    }

    Job *job = 0;
    for (int i = 0; i < pendingJobs.count(); ++i) {
        const QString jobDevice = pendingJobs[i]->device;
        if (!usedDevices.contains(jobDevice)) {
            usedDevices.append(jobDevice);
            job = pendingJobs.takeAt(i);
            break;
        }
    }

    if (!job)
        return;

    job->fix("/", "%2f");

    KUrl defaultTempDir;
    if (Prefs::enableTempDir()) {
        defaultTempDir = Prefs::tempDir();
    } else {
        defaultTempDir = KUrl(KStandardDirs::locateLocal("tmp", ""));
    }

    // For cases like "/tmp" where there is a missing /
    QString tempDirStr = defaultTempDir.path(KUrl::AddTrailingSlash);

    QString tmpFileName;
    do {
        tmpFileName = QString("%1%2_%3-%4_%5.wav").arg(tempDirStr).arg(job->track).arg(job->track_artist).arg(job->track_title).arg(KRandom::randomString(6));
    } while (QFile::exists(tmpFileName));
    
    QString n;
    // build the number like kio_audiocd, needs the same translation, I guess
    QString wavFile = QString("audiocd:/") + i18n("Track %1", n.sprintf("%02d", job->track)) + QString(".wav");

    KUrl source(wavFile);
    if (!job->device.isEmpty())
        source.addQueryItem("device", job->device);
    source.addQueryItem("fileNameTemplate", i18n("Track %1", QString("%{number}")));
    KUrl dest(tmpFileName);

    KIO::FileCopyJob *copyJob = KIO::file_copy(source, dest, 0644, KIO::HideProgressInfo);
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
void Ripper::copyJobResult(KJob *copyjob)
{
	if (!copyjob)
		return;
	KIO::FileCopyJob *copyJob = static_cast<KIO::FileCopyJob*> (copyjob);
	KNotification::event("track ripped");

	if(jobs.find(static_cast<KIO::Job*>(copyjob)) == jobs.end())
		return;
	Job *newJob = jobs[static_cast<KIO::Job*>(copyjob)];
	jobs.remove(static_cast<KIO::Job*>(copyjob));
	usedDevices.removeAll(newJob->device);

	if(Prefs::beepAfterRip())
		KNotification::beep();

	if ( copyJob->error() == 0 ) {
		emit updateProgress(newJob->id, JOB_COMPLETED);
		newJob->location = copyJob->destUrl().path();
        newJob->removeTempFile = Prefs::removeRippedWavs();
		emit(encodeWav(newJob));
	} else {
		copyJob->ui()->setWindow(0);
		copyJob->ui()->showErrorMessage();
		QFile file( (copyJob->destUrl()).path());
		file.remove();
		emit updateProgress(newJob->id, JOB_ERROR);
		delete newJob;
		newJob = 0;
	}

	if(newJob && newJob->lastSongInAlbum){
		if(Prefs::autoEjectAfterRip()){
			// Don't eject device if a pending job has that device
			Job *job = 0;
			foreach(Job *j, pendingJobs)
			{
				if( j->device == newJob->device )
				{
					job = j;
					break;
				}
			}
			if( !job ){
				deviceToEject = newJob->driveUdi;
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

