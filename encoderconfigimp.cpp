#include "encoderconfigimp.h"

#include <qapplication.h>
#include <kconfig.h>
#include <kglobal.h>
#include <qtimer.h>
#include <qspinbox.h>
#include <qlineedit.h>
#include <stdlib.h>
#include <qdir.h>
#include <kstddirs.h>
#include <qfile.h>
#include <qtextstream.h>
#include <qcheckbox.h>
#include <kmessagebox.h>
#include <klocale.h>
#include <kurl.h>
#include <qcombobox.h>
#include <qregexp.h>

#define ENCODER_LAME 0
#define ENCODER_WAV 1
#define ENCODER_OGG 2
#define ENCODER_OTHER 3

/**
 * A helper function to replace %X with the stuff in the album.
 * if slash it tru then put "" around the %X
 */
void replaceSpecialChars(QString &string, Job * job, bool slash){
  if(slash == true){
    string.replace(QRegExp("%album"), QString("\"%1\"").arg(job->album));
    string.replace(QRegExp("%genre"), QString("\"%1\"").arg(job->genre));
    string.replace(QRegExp("%artist"), QString("\"%1\"").arg(job->group));
    string.replace(QRegExp("%year"), QString("\"%1\"").arg(job->year));
    string.replace(QRegExp("%song"), QString("\"%1\"").arg(job->song));
    if( job->track < 10 )
      string.replace(QRegExp("%track"), QString("\"0%1\"").arg(job->track));
    else
      string.replace(QRegExp("%track"), QString("\"%1\"").arg(job->track));
    return;
  }
  else{
    string.replace(QRegExp("%album"), QString("%1").arg(job->album));
    string.replace(QRegExp("%genre"), QString("%1").arg(job->genre));
    string.replace(QRegExp("%artist"), QString("%1").arg(job->group));
    string.replace(QRegExp("%year"), QString("%1").arg(job->year));
    string.replace(QRegExp("%song"), QString("%1").arg(job->song));
    if( job->track < 10 )
      string.replace(QRegExp("%track"), QString("0%1").arg(job->track));
    else
      string.replace(QRegExp("%track"), QString("%1").arg(job->track));
    return;
  }
}

/**
 * Constructor, load settings.  Set the up the pull down menu with the correct item.
 */
EncoderConfigImp::EncoderConfigImp( QWidget* parent, const char* name):EncoderConfig(parent,name){
  connect(encoder, SIGNAL(activated(int)), this, SLOT(loadEncoderConfig(int)));

  KConfig &config = *KGlobal::config();
  config.setGroup("encoderconfig");
  deleteWav->setChecked(config.readBoolEntry("deleteWav", true));
  numberOfCpus->setValue(config.readNumEntry("numberOfCpus", 1));
  mp3FileFormat->setText(config.readEntry("mp3FileFormat", "~/mp3/%artist/%album/%artist - %song.mp3"));
  createM3uAlbum->setChecked(config.readBoolEntry("createM3uAlbum", false));
  m3uFileFormat->setText(config.readEntry("m3uFileFormat", "~/mp3/%artist/%album/%artist - %album.m3u"));
  useRelitivePath->setChecked(config.readBoolEntry("useRelitivePath", false));
  encoder->setCurrentItem(config.readNumEntry("encoder",0));
  loadEncoderConfig(encoder->currentItem());
}

/**
 * Deconstructor, remove pending jobs, remove current jobs, save settings.
 */
EncoderConfigImp::~EncoderConfigImp(){
  pendingJobs.clear();

  QMap<KShellProcess*, Job*>::Iterator it;
  for( it = jobs.begin(); it != jobs.end(); ++it ){
    KShellProcess *process = it.key();
    Job *job = jobs[it.key()];
    threads.remove(process);
    process->kill();
    QFile::remove(job->newLocation);
    delete job;
    delete process;
  }
  jobs.clear();

  KConfig &config = *KGlobal::config();
  config.setGroup("encoderconfig");
  config.writeEntry("encoder", encoder->currentItem());
  config.writeEntry("deleteWav", deleteWav->isChecked());
  config.writeEntry("numberOfCpus", numberOfCpus->value());
  config.writeEntry("mp3FileFormat", mp3FileFormat->text());
  config.writeEntry("createM3uAlbum", createM3uAlbum->isChecked());
  config.writeEntry("m3uFileFormat", m3uFileFormat->text());
  config.writeEntry("useRelitivePath", useRelitivePath->isChecked());
  if(encoder->currentItem() == ENCODER_LAME){
    config.writeEntry("encoderExeLame", encoderExe->text());
    config.writeEntry("encoderCommandLineLame", encoderCommandLine->text());
  }
  if(encoder->currentItem() == ENCODER_WAV){
    config.writeEntry("encoderExeWave", encoderExe->text());
    config.writeEntry("encoderCommandLineWav", encoderCommandLine->text());
  }
  if(encoder->currentItem() == ENCODER_OGG){
    config.writeEntry("encoderExeOggEnc", encoderExe->text());
    config.writeEntry("encoderCommandLineOggEnc", encoderCommandLine->text());
  }
  if(encoder->currentItem() == ENCODER_OTHER){
    config.writeEntry("encoderExeOther", encoderExe->text());
    config.writeEntry("encoderCommandLineOther", encoderCommandLine->text());
  }
}

/**
 * Load the settings for this encoder.
 * @param index the selected item in the drop down menu.
 */
void EncoderConfigImp::loadEncoderConfig(int index){
  KConfig &config = *KGlobal::config();
  config.setGroup("encoderconfig");

  if(index == ENCODER_LAME){
    encoderExe->setText(config.readEntry("encoderExeLame", "lame"));
    encoderCommandLine->setText(config.readEntry("encoderCommandLineLame", "--r3mix --tt %song --ta %artist --tl %album --ty %year --tn %track --tg %genre %f %o"));
    return;
  }
  if(index == ENCODER_WAV){
    encoderExe->setText(config.readEntry("encoderExeWave", "mv"));
    encoderCommandLine->setText(config.readEntry("encoderCommandLineWav", "%f %o"));
    return;
  }
  if(index == ENCODER_OGG){
    encoderExe->setText(config.readEntry("encoderExeOggEnc", "oggenc"));
    encoderCommandLine->setText(config.readEntry("encoderCommandLineOggEnc", "-o %o -a %artist -l %album -t %song -N %track %f"));
     return;
  }
  if(index == ENCODER_OTHER){
    encoderExe->setText(config.readEntry("encoderExeOther", ""));
    encoderCommandLine->setText(config.readEntry("encoderCommandLineOther", ""));
    return;
  }
}

/**
 * Stop this job with the matchin id.
 * @param id the id number of the job to stop.
 */
void EncoderConfigImp::removeJob(int id){
  QMap<KShellProcess*, Job*>::Iterator it;
  for( it = jobs.begin(); it != jobs.end(); ++it ){
    if(it.data()->id == id){
      KShellProcess *process = it.key();
      Job *job = jobs[it.key()];
      threads.remove(process);
      process->kill();
      jobs.remove(process);
      delete job;
      delete process;
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
}

/**
 * Adds job to the que of jobs to encode.
 * @param job the job to encode.
 */
void EncoderConfigImp::encodeWav(Job *job){
  emit(addJob(job, i18n("Encoding: %1 - %2").arg(job->group).arg(job->song)));
  pendingJobs.append(job);
  tendToNewJobs();
}

/**
 * See if there are are new jobs to attend too.  If we are all loaded up
 * then just loop back in 5 seconds and check agian.
 */
void EncoderConfigImp::tendToNewJobs(){
  while(threads.count() >= (uint)numberOfCpus->value()){
    QTimer::singleShot( threads.count()*2*1000, this, SLOT(tendToNewJobs()));
    return;
  }
  if(pendingJobs.count() == 0)
    return;
 
  Job *job = pendingJobs.first();
  pendingJobs.remove(job);
  job->jobType = encoder->currentItem();

  QString desiredFile = mp3FileFormat->text();
  replaceSpecialChars(desiredFile, job, false);
  if(desiredFile[0] == '~'){
    desiredFile.replace(0,1, QDir::homeDirPath());
  }
  int lastSlash = desiredFile.findRev('/',-1);
  if( lastSlash == -1 || !(KStandardDirs::makeDir( desiredFile.mid(0,lastSlash)))){
   qDebug("Can not place file, unable to make directorys");
    return;
  }

  job->newLocation = desiredFile;
  
  QString command = encoderExe->text();
  command += " " + encoderCommandLine->text();
  replaceSpecialChars(command, job, true);
  command.replace(QRegExp("%f"), "\"" + job->location + "\"");
  command.replace(QRegExp("%o"), "\"" + desiredFile + "\"");
  
  // WHY O WHY DOES THIS HAVE TO BE HERE, WHY CAN'T AUDIOCD:/ MAKE IT +r?
  system(QString("chmod +r \"%1\"").arg(job->location).latin1());
  updateProgress(job->id, 1);
  
  job->errorString = command;
  //qDebug(command.latin1());
  KShellProcess *proc = new KShellProcess();
  //*proc << "bash" << "-c" << command.latin1();
  *proc << command.latin1();
  connect(proc, SIGNAL(receivedStdout(KProcess *, char *, int )),
                       this, SLOT(receivedThreadOutput(KProcess *, char *, int )));
  connect(proc, SIGNAL(receivedStderr(KProcess *, char *, int )),
                       this, SLOT(receivedThreadOutput(KProcess *, char *, int )));
  connect(proc, SIGNAL(processExited(KProcess *)), this, SLOT(jobDone(KProcess *)));
  jobs.insert(proc, job);
  threads.append(proc);

  proc->start(KShellProcess::NotifyOnExit,  KShellProcess::AllOutput);
}

/**
 * We have recieved some output from a thread. See if it contains a foo%.
 * @param proc the process that has new output. 
 * @param buffer the output from the process
 * @param buflen the length of the buffer.
 */
void EncoderConfigImp::receivedThreadOutput(KProcess *process, char *buffer, int){
  QString output = buffer;
  int percent = output.find('%');
  if(percent!=-1){
    output = output.mid(percent-2,2);

    Job *job = jobs[(KShellProcess*)process];
    if(job){
      if(job->jobType == ENCODER_OGG) // oggenc outputs 'xx.y%'
        output = output.mid(percent-4,2);
    }
    else
      output = output.mid(percent-2,2);
    int percent = output.toInt();
    if(percent > 0 && percent < 100){
      Job *job = jobs[(KShellProcess*)process];
      if(job)
        emit(updateProgress(job->id, percent));
    }
  }
} 

/**
 * When the thread is done encoding the file this function is called.
 * @param job the job that just finished.
 */
void EncoderConfigImp::jobDone(KProcess *process){
  if(!process)
    return;

  Job *job = jobs[(KShellProcess*)process];
  threads.remove((KShellProcess*)process);
  jobs.remove((KShellProcess*)process);

  if( QFile::exists(job->newLocation)){
    if(deleteWav->isChecked())
      QFile::remove(job->location);
    emit(updateProgress(job->id, 100));
    if(createM3uAlbum->isChecked())
      appendToPlaylist(job);
  }
  else{
    KMessageBox::sorry(this, i18n("The encoded file was not created.\nPlease check your encoder options.\nThe wav file has been removed.  Command was: %1").arg(job->errorString), i18n("Encoding Failed"));
    QFile::remove(job->location);
    emit(updateProgress(job->id, -1));
  }

  delete job;
  delete process;
}

/**
 * Append the job to the playlist as specified in the options.
 * @param job too append to the playlist.
 */
void EncoderConfigImp::appendToPlaylist(Job* job){
  QString desiredFile = m3uFileFormat->text();
  replaceSpecialChars(desiredFile, job, false);
  if(desiredFile[0] == '~'){
    desiredFile.replace(0,1, QDir::homeDirPath());
  }
  int lastSlash = desiredFile.findRev('/',-1);
  if( lastSlash == -1 || !(KStandardDirs::makeDir( desiredFile.mid(0,lastSlash)))){
    KMessageBox::sorry(this, i18n("The desired encoded file could not be created.\nPlease check your file path option.\nThe wav file has been removed."), i18n("Encoding Failed"));
    QFile::remove(job->location);
    return;
  }

  QFile f(desiredFile);
  if ( !f.open(IO_WriteOnly|IO_Append) ){
    KMessageBox::sorry(this, i18n("The desired playlist file could not be opened for writing to.\nPlease check your file path option."), i18n("Playlist addition Failed"));
    return;
  }

  QTextStream t( &f );        // use a text stream

  if(useRelitivePath->isChecked()){
    KURL audioFile(job->newLocation);
    t << "./" << audioFile.fileName() << endl;
  }
  else{
    t << job->newLocation << endl;
  }
  f.close();

}

#include "encoderconfigimp.moc"

// encoderconfigimp.cpp

