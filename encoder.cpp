#include "encoder.h"

#include <qregexp.h>
#include <qtimer.h>
#include <qdir.h>
#include <kstandarddirs.h>
#include <kmessagebox.h>
#include <kurl.h>
#include <kconfig.h>

/**
 * Constructor, load settings.
 */
Encoder::Encoder( QObject* parent, const char* name):QObject(parent,name) {
  loadSettings();
}

/**
 * Load the settings for this class.
 */ 
void Encoder::loadSettings(){
  KConfig &config = *KGlobal::config();
  
  config.setGroup("General");
  replaceInput = config.readEntry("selection");
  replaceOutput = config.readEntry("replace");
  
  config.setGroup("Encoder");
  uint currentEncoder = config.readNumEntry("encoderChoice", 0);
  QString currentEncoderGroup = QString("Encoder_%1").arg(currentEncoder);
  if(!config.hasGroup(currentEncoderGroup)){
    KMessageBox::sorry(0, i18n("No encoder has been selected.\nPlease select an encoder in the configuration."), i18n("No Encoder Selected"));
  }
  else{
    config.setGroup(currentEncoderGroup);
    encoderCommandLine = config.readPathEntry("commandLine");
    encoderExtension = config.readEntry("extension");
    encoderPercentLength = config.readNumEntry("percentLength");
  }
	  
  config.setGroup("Encoder");
  numberOfCpus = config.readNumEntry("numberOfCpus", 1);
  fileFormat = config.readPathEntry("fileFormat", "~/%extension/%artist/%album/%artist - %song.%extension");
  createPlaylist = config.readBoolEntry("createPlaylist", false);
  playlistFileFormat = config.readPathEntry("playlistFileFormat", "~/%extension/%artist/%album/%artist - %album.m3u");
  useRelitivePath = config.readBoolEntry("useRelitivePath", false);
}

/**
 * Deconstructor, remove pending jobs, remove current jobs.
 */
Encoder::~Encoder(){
  pendingJobs.clear();

  QMap<KShellProcess*, Job*>::Iterator pit;
  for( pit = jobs.begin(); pit != jobs.end(); ++pit ){
    Job *job = jobs[pit.key()];
    KShellProcess *process = pit.key();
    threads.remove(process);
    process->kill();
    QFile::remove(job->newLocation);
    delete job;
    delete process;
  }
  jobs.clear();
}

/**
 * Stop this job with the matching id.
 * @param id the id number of the job to stop.
 */
void Encoder::removeJob(int id){
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
void Encoder::encodeWav(Job *job){
  emit(addJob(job, i18n("Encoding (%1): %2 - %3").arg(encoderExtension).arg(job->group).arg(job->song)));
  pendingJobs.append(job);
  tendToNewJobs();
}

/**
 * See if there are are new jobs to attend too.  If we are all loaded up
 * then just loop back in a few seconds and check agian.
 */
void Encoder::tendToNewJobs(){
  // If we are currently ripping the max try again in a little bit.
  if(threads.count() >= numberOfCpus){
    QTimer::singleShot( (threads.count()+1)*2*1000, this, SLOT(tendToNewJobs()));
    return;
  }
  // Just to make sure in the event something goes wrong or we are exiting
  if(pendingJobs.count() == 0)
    return;

  Job *job = pendingJobs.first();
  pendingJobs.remove(job);

  QString desiredFile = fileFormat;
  {
    QMap <QString,QString> map;
    map.insert("extension", encoderExtension);
    job->replaceSpecialChars(desiredFile, false, map);
  }
  if(desiredFile[0] == '~'){
    desiredFile.replace(0,1, QDir::homeDirPath());
  }

  // If the user wants anything regexp replaced do it now...
  desiredFile.replace( QRegExp(replaceInput), replaceOutput );
  
  int lastSlash = desiredFile.findRev('/',-1);
  if( lastSlash == -1 || !(KStandardDirs::makeDir( desiredFile.mid(0,lastSlash)))){
    qDebug("Can not place file, unable to make directories.");
    return;
  }

  job->newLocation = desiredFile;

  QString command = encoderCommandLine;
  {
    QMap <QString,QString> map;
    map.insert("extension", encoderExtension);
    map.insert("f", job->location);
    map.insert("o", desiredFile);
    job->replaceSpecialChars(command, true, map);
  }

  updateProgress(job->id, 1);

  job->errorString = command;
  KShellProcess *proc = new KShellProcess();
  *proc << QFile::encodeName(command);
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
 * We have recieved some output from a thread. See if it contains %.
 * @param proc the process that has new output.
 * @param buffer the output from the process
 * @param buflen the length of the buffer.
 */
void Encoder::receivedThreadOutput(KProcess *process, char *buffer, int length){
  // Make sure we have a job to send an update too.
  Job *job = jobs[(KShellProcess*)process];
  if(!job){
    qDebug("Encoder::receivedThreadOutput Job doesn't exists. Line: %d", __LINE__);
    return;
  }

  // Make sure the output string has a % symble in it.
  QString output = QString(buffer).mid(0,length);
  if( output.find('%') == -1 ){
    qDebug("No \'%%\' in output.  Report as bug w/encoder options if progressbar doesn't fill.");
    return;
  }
  //qDebug(QString("Pre cropped: %1").arg(output).latin1());
  output = output.mid(output.find('%')-encoderPercentLength,2);
  //qDebug(QString("Post cropped: %1").arg(output).latin1());
  bool conversionSuccessfull = false;
  int percent = output.toInt(&conversionSuccessfull);
  //qDebug(QString("number: %1").arg(percent).latin1());
  if(percent >= 0 && percent < 100 && conversionSuccessfull){
    emit(updateProgress(job->id, percent));
  }
  // If it was just some random output that couldn't be converted then don't report the error.
  else
    if(conversionSuccessfull)
      qDebug("Percent done:\"%d\" is not >= 0 && < 100.", percent);
}

/**
 * When the process is done encoding a file this function is called.
 * @param job the job that just finished.
 */
void Encoder::jobDone(KProcess *process){
  // Normal error checking here.
  if(!process)
    return;
 
  //qDebug("Process exited with status: %d", process->exitStatus());
  
  Job *job = jobs[(KShellProcess*)process];
  threads.remove((KShellProcess*)process);
  jobs.remove((KShellProcess*)process);

  if(process->exitStatus() == 127){
    KMessageBox::sorry(0, i18n("The selected encoder was not found.\nThe wav file has been removed. Command was: %1").arg(job->errorString), i18n("Encoding Failed"));
    QFile::remove(job->location);
    emit(updateProgress(job->id, -1));
  }
  else if( QFile::exists(job->newLocation)){
    QFile::remove(job->location);
    
    // TODO kill -9 lame or oggenc when processing and see what they return.
    if(process->normalExit() && process->exitStatus() != 0){
      //qDebug("Failed to complete!");
      qDebug("Process exited with non 0 status: %d", process->exitStatus());
    }
    else{ 
      //qDebug("Must be done: %d", (process->exitStatus()));
      emit(updateProgress(job->id, 100));
    }
    if(createPlaylist)
      appendToPlaylist(job);
  }
  else{
    KMessageBox::sorry(0, i18n("The encoded file was not created.\nPlease check the encoder options.\nThe wav file has been removed. Command was: %1").arg(job->errorString), i18n("Encoding Failed"));
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
void Encoder::appendToPlaylist(Job* job){
  QString desiredFile = playlistFileFormat;
  QMap <QString,QString> map;
  map.insert("extension", encoderExtension);
  job->replaceSpecialChars(desiredFile, false, map);
  if(desiredFile[0] == '~'){
    desiredFile.replace(0,1, QDir::homeDirPath());
  }
  int lastSlash = desiredFile.findRev('/',-1);
  if( lastSlash == -1 || !(KStandardDirs::makeDir( desiredFile.mid(0,lastSlash)))){
    KMessageBox::sorry(0, i18n("The desired playlist file could not be created.\nPlease check the set path.\n"), i18n("Playlist Creation Failed"));
    return;
  }

  QFile f(desiredFile);
  if ( !f.open(IO_WriteOnly|IO_Append) ){
    KMessageBox::sorry(0, i18n("The desired playlist file could not be opened for writing to.\nPlease check the file path option."), i18n("Playlist Addition Failed"));
    return;
  }

  QTextStream t( &f );        // use a text stream

  bool relWorked = false;
  if(useRelitivePath){
    KURL audioFile(job->newLocation);
    QString relative;
    KURL d(desiredFile);
    relWorked = relativeURL(d.directory(), audioFile.directory(), relative);
    if(relWorked)
      t << relative << audioFile.fileName() << endl;
  }
  if(!relWorked)
    t << job->newLocation << endl;
  
  f.close();
}

/**
 * Finds the relative path from path1 to path2
 * returns true is successfull.
 */
bool Encoder::relativeURL(const QString &path1, const QString &path2,
				QString &relativePath ) const {
  QDir p1(QDir::cleanDirPath(path1));
  QDir p2(QDir::cleanDirPath(path2));
  if(!p1.exists() || !p2.exists())
    return false;
 
  // If they are the same
  if(p1.path() == p2.path()){
    relativePath = "./";
    return true;
  }
  
  QStringList list1 = QStringList::split('/', p1.path());
  QStringList list2 = QStringList::split('/', p2.path());
 
  // Find where they meet
  uint level = 0;
  while(list1[level] == list2[++level]);

  // Need to go down out of the first path to the common branch.
  for(uint i=level; i < list1.count(); i++)
    relativePath += "../";
 
  // Now up up from the common branch to the second path.
  for(; level < list2.count(); level++)
    relativePath += list2[level] + "/";
  return true;
}

#include "encoder.moc"

