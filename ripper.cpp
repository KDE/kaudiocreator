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

#include "ripper.h"

#include "prefs.h"

#include <qfile.h>
#include <qtimer.h>
#include <ktempfile.h>

// eject
#include <kmessagebox.h>

// beep
#include <knotifyclient.h>

/**
 * Constructor, load settings.
 */
Ripper::Ripper( QObject* parent, const char* name) : QObject(parent,name) {
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
    if(job){
      delete job;
    }
    if(ioJob){
      KIO::FileCopyJob *copyJob = dynamic_cast<KIO::FileCopyJob*> (ioJob);
      disconnect(copyJob, SIGNAL(result(KIO::Job*)), this, SLOT(copyJobResult(KIO::Job*)));
      disconnect(copyJob, SIGNAL(percent ( KIO::Job *, unsigned long)), this, SLOT(updateProgress ( KIO::Job *, unsigned long)));
      QString fileDestination = (copyJob->destURL()).path();
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
      QString fileDestination = (copyJob->destURL()).path();
      copyJob->kill();
      
      // This here is such a hack, shouldn't kill() do this, or why isn't there a stop()?
      // TODO add to copyJob a stop() function.
      QFile file( fileDestination );
      if(file.exists())
        file.remove();
      else{
        QFile f( fileDestination+".part" );
        f.remove();
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
  emit(addJob(job, i18n("Ripping: %1 - %2").arg(job->group).arg(job->song)));
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
  pendingJobs.remove(job);

  QMap<QString, QString> map;
  QString defaultTempDir;
  if(Prefs::enableTempDir())
    defaultTempDir = Prefs::tempDir();
  KTempFile tmp( defaultTempDir );
  tmp.setAutoDelete(true);

  QString wavFile;
  QString jobDevice = job->device;
  if(!jobDevice.isEmpty())
    jobDevice = QString("?device=%1").arg(jobDevice);
  if(job->track < 10)
    wavFile = QString("audiocd:/By Track/Track 0%1.wav%2").arg(job->track).arg(jobDevice); //lukas: I fear this won't work (Works for kde2)
  else
    wavFile = QString("audiocd:/By Track/Track %1.wav%2").arg(job->track).arg(jobDevice);

  KURL source(wavFile);
  KURL dest(tmp.name());

  KIO::FileCopyJob *copyJob = new KIO::FileCopyJob(source, dest, 0664, FALSE, TRUE, FALSE, FALSE);
  connect(copyJob, SIGNAL(result(KIO::Job*)), this, SLOT(copyJobResult(KIO::Job*)));
  connect(copyJob, SIGNAL(percent ( KIO::Job *, unsigned long)), this, SLOT(updateProgress ( KIO::Job *, unsigned long)));
  jobs.insert(copyJob, job);
  emit jobsChanged();
}

/**
 * Copies the data from the KIO Job.  Uses this data to fill in the
 * information dialog.
 * @param job the IO job to copy from
 */
void Ripper::copyJobResult(KIO::Job *job){
  KIO::FileCopyJob *copyJob = dynamic_cast<KIO::FileCopyJob*> (job);

  Job *newJob = jobs[job];
  jobs.remove(job);

  if(Prefs::beepAfterRip())
    KNotifyClient::beep();
   
  if ( copyJob->error() == 0 ){
    emit updateProgress(newJob->id, 100);
    newJob->location = copyJob->destURL().path();
    emit( encodeWav(newJob));
  }
  else{
    copyJob->showErrorDialog(0);
    QFile file( (copyJob->destURL()).path());
    file.remove();
    emit updateProgress(newJob->id, -1);
    delete newJob;
  }

  if(newJob->lastSongInAlbum){
    if(Prefs::autoEjectAfterRip()){
      QTimer::singleShot( Prefs::autoEjectDelay()*1000 + 500, this, SIGNAL(eject()));
    }
  }
  tendToNewJobs();
}

/**
 * Update the progress of the file copy.
 * @param job the current ioslave job in progress
 * @param percent the current percent that the ioslave has done.
 */
void Ripper::updateProgress( KIO::Job *job, unsigned long percent){
  if(job){
    Job *ripJob = (jobs[job]);
    if(ripJob)
      emit updateProgress(ripJob->id, percent);
  }
}

#include "ripper.moc"

