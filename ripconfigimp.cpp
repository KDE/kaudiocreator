#include "ripconfigimp.h"

#include "job.h"
#include <qfile.h>
#include <ktempfile.h>
#include <qfileinfo.h>
#include <qglobal.h>
#if QT_VERSION < 300
#include <kapp.h>
#else
#include <kapplication.h>
#endif
#include <qcheckbox.h>
#include <qspinbox.h>
#include <qtimer.h>
#include <qregexp.h>
#include <klineedit.h>
#include <kstandarddirs.h>
#include <qdir.h>
#include <kconfig.h>
#include <stdlib.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <knotifyclient.h>

/**
 * Constructor, load settings.
 */
RipConfigImp::RipConfigImp( QWidget* parent, const char* name):RipConfig(parent,name){
  KConfig &config = *KGlobal::config();
  config.setGroup("ripconfig");
  fileFormat->setText(config.readEntry("fileFormat", "~/wav/%artist/%album/%artist - %song.wav"));
  maxWavFiles->setValue(config.readNumEntry("maxWavFiles", 1));
  beepAfterRip->setChecked(config.readBoolEntry("beepAfterRip", true));
  autoEjectAfterRip->setChecked(config.readBoolEntry("autoEjectAfterRip", false));
  autoEjectDelay->setValue(config.readNumEntry("autoEjectDelay", 0));
}

/**
 * Deconstructor, remove pending jobs, remove current jobs, save settings.
 */
RipConfigImp::~RipConfigImp(){
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

  KConfig &config = *KGlobal::config();
  config.setGroup("ripconfig");
  config.writeEntry("fileFormat", fileFormat->text());
  config.writeEntry("maxWavFiles",maxWavFiles->value());
  config.writeEntry("beepAfterRip", beepAfterRip->isChecked());
  config.writeEntry("autoEjectAfterRip", autoEjectAfterRip->isChecked());
  config.writeEntry("autoEjectDelay", autoEjectDelay->value());
}

/**
 * Cancel and remove the job with the matching id.
 * Remove it from the local collection of jobs, delete the temp file if
 * there is one.
 * @param id the id number of the job to remove.
 */
void RipConfigImp::removeJob(int id){
  QMap<KIO::Job*, Job*>::Iterator it;
  for( it = jobs.begin(); it != jobs.end(); ++it ){
    if(it.data()->id == id){
      jobs.remove(it.key());
      KIO::FileCopyJob *copyJob = dynamic_cast<KIO::FileCopyJob*> (it.key());
      QString fileDestination = (copyJob->destURL()).path();
      copyJob->kill();
      QFile file( fileDestination );
      file.remove();
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
}

/**
 * Begin to rip the track specified in job.
 * @param job the new job that this module should take over.
 * @param job the new job that we need to handle.
 */
void RipConfigImp::ripTrack(Job *job){
  emit(addJob(job, i18n("Ripping: %1 - %2").arg(job->group).arg(job->song)));
  pendingJobs.append(job);
  tendToNewJobs();
}

/**
 * See if there are are new jobs to attend too.  If we are all loaded up
 * then just loop.
 */
void RipConfigImp::tendToNewJobs(){
  // If we are currently ripping the max try again in a little bit.
  if(jobs.count() >= (uint)maxWavFiles->value()){
    QTimer::singleShot( (jobs.count()+1)*2*1000, this, SLOT(tendToNewJobs()));
    return;
  }
  // Just to make sure in the event something goes wrong
  if(pendingJobs.count() == 0)
    return;

  Job *job = pendingJobs.first();
  pendingJobs.remove(job);

  QString desiredFile = fileFormat->text();
  desiredFile.replace(QRegExp("%album"), job->album);
  desiredFile.replace(QRegExp("%genre"), job->genre);
  desiredFile.replace(QRegExp("%artist"), job->group);
  desiredFile.replace(QRegExp("%year"), QString("%1").arg(job->year));
  desiredFile.replace(QRegExp("%song"), job->song);
  if( job->track < 10 )
    desiredFile.replace(QRegExp("%track"), QString("\"0%1\"").arg(job->track));
  else
    desiredFile.replace(QRegExp("%track"), QString("\"%1\"").arg(job->track));

  if(desiredFile[0] == '~'){
    desiredFile.replace(0,1, QDir::homeDirPath());
  }

  int lastSlash = desiredFile.findRev('/',-1);
  if( !(KStandardDirs::makeDir( desiredFile.mid(0,lastSlash)))){
    KMessageBox::sorry(this, i18n("The desired ripping file could not created.\nPlease check your file path option."), i18n("Ripping Failed"));
    return;
  }

  QString wavFile;
  #if QT_VERSION < 300
  if(1)
  #else
  if(job->track < 10)
  #endif
    wavFile = QString("audiocd:/By Track/Track 0%1.wav").arg(job->track); //lukas: I fear this won't work
  else
    wavFile = QString("audiocd:/By Track/Track %1.wav").arg(job->track);

  KURL source(wavFile);
  KURL dest(desiredFile);

  KIO::FileCopyJob *copyJob = new KIO::FileCopyJob(source, dest, 0664, FALSE, TRUE, FALSE, FALSE);
  connect(copyJob, SIGNAL(result(KIO::Job*)), this, SLOT(copyJobResult(KIO::Job*)));
  connect(copyJob, SIGNAL(percent ( KIO::Job *, unsigned long)), this, SLOT(updateProgress ( KIO::Job *, unsigned long)));
  jobs.insert(copyJob, job);

}

/**
 * Copies the data from the KIO Job.  Uses this data to fill in the
 * information dialog.
 * @param job the IO job to copy from
 */
void RipConfigImp::copyJobResult(KIO::Job *job){
  KIO::FileCopyJob *copyJob = dynamic_cast<KIO::FileCopyJob*> (job);

  Job *newJob = jobs[job];
  jobs.remove(job);

  if ( copyJob->error() == 0 ){
    emit updateProgress(newJob->id, 100);
    QString newFileLocation = copyJob->destURL().path();
    newJob->location = newFileLocation;
    emit( encodeWav(newJob));
  }
  else{
    copyJob->showErrorDialog(this);
    QFile file( (copyJob->destURL()).path());
    file.remove();
    emit updateProgress(newJob->id, -1);
    delete newJob;
  }

  if(newJob->lastSongInAlbum){
    if(autoEjectAfterRip->isChecked())
      QTimer::singleShot( autoEjectDelay->value()*1000, this, SLOT(eject()));
  }
  if(beepAfterRip->isChecked()){
    KNotifyClient::beep();
  }
}

/**
 * Eject the cd
 */
void RipConfigImp::eject(){
  system("eject");
}

/**
 * Update the progress of the file copy.
 * @param job the current ioslave job in progress
 * @param percent the current percent that the ioslave has done.
 */
void RipConfigImp::updateProgress( KIO::Job *job, unsigned long percent){
  if(job){
    Job *ripJob = (jobs[job]);
    if(ripJob)
      emit updateProgress(ripJob->id, percent);
  }
}

#include "ripconfigimp.moc"

// ripconfigimp.cpp

