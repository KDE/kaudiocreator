#include "encoderconfigimp.h"
#include "wizard.h"

#include <qapplication.h>
#include <qpushbutton.h>
#include <qspinbox.h>
#include <qlineedit.h>
#include <qcheckbox.h>
#include <qcombobox.h>
#include <qregexp.h>
#include <qtimer.h>
#include <qfile.h>
#include <qtextstream.h>
#include <qdir.h>
#include <kstandarddirs.h>
#include <kmessagebox.h>
#include <klocale.h>
#include <kurl.h>
#include <kconfig.h>
#include <kglobal.h>

#include "kmacroexpander.h"

#define ENCODER_EXE_STRING "encoderExe_"
#define ENCODER_ARGS_STRING "encoderCommandLine_"
#define ENCODER_EXTENSION_STRING "encoderExtension_"
#define ENCODER_PERCENTLENGTH_STRING "encoderPercentLength_"

// Clean up the string so that it doesn't wander off to unexpected directories
static QString sanitize(const QString &s)
{
  QString result = s;
  result.replace('/', ":");
  if (result.isEmpty())
    result = "(empty)";
  if (result[0] == '.')
    result[0] = '_';
  return result;
}

/**
 * A helper function to replace %X with the stuff in the album.
 * if quote is true then put "" around the %X
 */
void EncoderConfigImp::replaceSpecialChars(QString &string, Job * job, bool quote, QMap<QString, QString> _map){
  QMap<QString,QString> map = _map;
  
  map.insert("album", sanitize(job->album));
  map.insert("genre", sanitize(job->genre));
  map.insert("artist", sanitize(job->group));
  map.insert("year", QString::number(job->year));
  map.insert("song", sanitize(job->song));
  map.insert("extension", sanitize(encoderExtensionLineEdit->text()));
  if( job->track < 10 )
      map.insert("track", "0" + QString::number(job->track) );
  else
      map.insert("track", QString::number(job->track) );

  if (quote)
      KAudioCreator::KSelfDelimitingMacroMapExpander::expandMacrosShellQuote(string, map);
  else
      KAudioCreator::KSelfDelimitingMacroMapExpander::expandMacros(string, map);
  
}

/**
 * Constructor, load settings.  Set the up the pull down menu with the correct item.
 */
EncoderConfigImp::EncoderConfigImp( QWidget* parent, const char* name):EncoderConfig(parent,name), encodersPercentStringLength(2), oldEncoderSelection(-1), save(false){
  connect(encoder, SIGNAL(activated(int)), this, SLOT(loadEncoderConfig(int)));
  connect(playlistWizardButton, SIGNAL(clicked()), this, SLOT(playlistWizard()));
  connect(encoderWizardButton, SIGNAL(clicked()), this, SLOT(encoderWizard()));

  KConfig &config = *KGlobal::config();
  config.setGroup("encoderconfig");
  saveWav->setChecked(config.readBoolEntry("saveWav", false));
  numberOfCpus->setValue(config.readNumEntry("numberOfCpus", 1));
  fileFormat->setText(config.readEntry("fileFormat", "~/%extension/%artist/%album/%artist - %song.%extension"));
  createPlaylistCheckBox->setChecked(config.readBoolEntry("createPlaylist", false));
  playlistFileFormat->setText(config.readEntry("playlistFileFormat", "~/%extension/%artist/%album/%artist - %album.m3u"));
  useRelitivePath->setChecked(config.readBoolEntry("useRelitivePath", false));

  int totalNumberOfEncoders = config.readNumEntry("numberOfEncoders",0);
  if( totalNumberOfEncoders == 0){
    encoderName.insert(0, i18n("Lame"));
    encoder->insertItem(i18n("Lame"));
    encoderArgs.insert(0, "lame --r3mix --tt %song --ta %artist --tl %album --ty %year --tn %track --tg %genre %f %o");
    encoderExtension.insert(0, "mp3");
    encoderpercentLength.insert(0, 2);

    encoderName.insert(1, i18n("OggEnc"));
    encoder->insertItem(i18n("OggEnc"));
    encoderArgs.insert(1, "oggenc -o %o -a %artist -l %album -t %song -N %track %f");
    encoderExtension.insert(1, "ogg");
    encoderpercentLength.insert(1, 4);

    encoderName.insert(2, i18n("Leave as Wav"));
    encoder->insertItem(i18n("Leave as Wav"));
    encoderArgs.insert(2, "mv %f %o");
    encoderExtension.insert(2, "wav");
    encoderpercentLength.insert(2, 2);

    encoderName.insert(3, i18n("Other"));
    encoder->insertItem(i18n("Other"));
    encoderArgs.insert(3, "");
    encoderExtension.insert(3, "");
    encoderpercentLength.insert(3, 2);
  }

  /***
   * The Encoders can be entirly loaded and are not hard coded.  You can add remove them on the fly
   * using the configure file, but a set of default ones are include here.
   *
   * If you would like to add a default value to here contact ben@meyerhome.net with apropriate values
   *
   * Encoder name - name of the encoder
   * Arguments - Command line string to encoder a file using that encoder
   * Extension - File extension that is generated.
   * Percent output length.  99.00% == 4, 99.9% == 3, 99% == 2
   */
  for(int i=0; i < totalNumberOfEncoders; i++){
    encoderName.insert(i, config.readEntry(QString(ENCODER_EXE_STRING "%1").arg(i),""));
    encoder->insertItem(config.readEntry(QString(ENCODER_EXE_STRING "%1").arg(i),""),i);
    encoderArgs.insert(i, config.readEntry(QString(ENCODER_ARGS_STRING "%1").arg(i),""));
    encoderExtension.insert(i, config.readEntry(QString(ENCODER_EXTENSION_STRING "%1").arg(i),""));
    encoderpercentLength.insert(i, config.readNumEntry(QString(ENCODER_PERCENTLENGTH_STRING "%1").arg(i),2));
  }

  // Set the current item and settings.
  int currentItem = config.readNumEntry("encoderCurrentItem",0);
  encoder->setCurrentItem(currentItem);
  loadEncoderConfig(encoder->currentItem());

}

/**
 * Deconstructor, remove pending jobs, remove current jobs, save settings.
 */
EncoderConfigImp::~EncoderConfigImp(){
  pendingJobs.clear();

  QMap<KShellProcess*, Job*>::Iterator pit;
  for( pit = jobs.begin(); pit != jobs.end(); ++pit ){

    KShellProcess *process = pit.key();
    Job *job = jobs[pit.key()];
    threads.remove(process);
    process->kill();
    QFile::remove(job->newLocation);
    delete job;
    delete process;
  }
  jobs.clear();

  KConfig &config = *KGlobal::config();
  config.setGroup("encoderconfig");
  config.writeEntry("encoderCurrentItem", encoder->currentItem());
  config.writeEntry("saveWav", saveWav->isChecked());
  config.writeEntry("numberOfCpus", numberOfCpus->value());
  config.writeEntry("fileFormat", fileFormat->text());
  config.writeEntry("createPlaylist", createPlaylistCheckBox->isChecked());
  config.writeEntry("playlistFileFormat", playlistFileFormat->text());
  config.writeEntry("useRelitivePath", useRelitivePath->isChecked());

  if(!save)
    return;
  QMap<int, QString>::Iterator it;
  for( it = encoderName.begin(); it != encoderName.end(); ++it )
    config.writeEntry(QString(ENCODER_EXE_STRING "%1").arg(it.key()), it.data());
  for( it = encoderArgs.begin(); it != encoderArgs.end(); ++it )
    config.writeEntry(QString(ENCODER_ARGS_STRING "%1").arg(it.key()), it.data());
  for( it = encoderExtension.begin(); it != encoderExtension.end(); ++it )
    config.writeEntry(QString(ENCODER_EXTENSION_STRING "%1").arg(it.key()), it.data());
  QMap<int, int>::Iterator nit;
  for( nit = encoderpercentLength.begin(); nit != encoderpercentLength.end(); ++nit )
    config.writeEntry(QString(ENCODER_PERCENTLENGTH_STRING "%1").arg(nit.key()), nit.data());

  config.writeEntry("encoderCurrentItem",encoder->currentItem());
  config.writeEntry("numberOfEncoders", encoder->count());
}

/**
 * Load the settings for this encoder.
 * @param index the selected item in the drop down menu.
 */
void EncoderConfigImp::loadEncoderConfig(int index){
  if(encoderArgs[oldEncoderSelection] != encoderCommandLine->text() && oldEncoderSelection != -1){
    //encoderArgs.remove(index);
    encoderArgs.insert(oldEncoderSelection, encoderCommandLine->text());
    save = true;
  }
  if(encoderExtension[oldEncoderSelection] != encoderExtensionLineEdit->text() && oldEncoderSelection != -1){
    //encoderExtension.remove(index);
    encoderExtension.insert(oldEncoderSelection, encoderExtensionLineEdit->text());
    save = true;
  }
  oldEncoderSelection = index;

  // Now you can load the new settings.
  encoderCommandLine->setText(encoderArgs[index]);
  encoderExtensionLineEdit->setText(encoderExtension[index]);
  encodersPercentStringLength = encoderpercentLength[index];
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
  // If we are currently ripping the max try again in a little bit.
  if(threads.count() >= (uint)numberOfCpus->value()){
    QTimer::singleShot( (threads.count()+1)*2*1000, this, SLOT(tendToNewJobs()));
    return;
  }
  // Just to make sure in the event something goes wrong
  if(pendingJobs.count() == 0)
    return;

  Job *job = pendingJobs.first();
  pendingJobs.remove(job);
  job->jobType = encoder->currentItem();

  QString desiredFile = fileFormat->text();
  {
    QMap <QString,QString> map;
    replaceSpecialChars(desiredFile, job, false, map);
  }
  if(desiredFile[0] == '~'){
    desiredFile.replace(0,1, QDir::homeDirPath());
  }
  int lastSlash = desiredFile.findRev('/',-1);
  if( lastSlash == -1 || !(KStandardDirs::makeDir( desiredFile.mid(0,lastSlash)))){
   qDebug("Can not place file, unable to make directories");
    return;
  }

  job->newLocation = desiredFile;

  QString command = encoderCommandLine->text();
  {
    QMap <QString,QString> map;
    map.insert("f", job->location);
    map.insert("o", desiredFile);
    replaceSpecialChars(command, job, true, map);
  }

  updateProgress(job->id, 1);

  job->errorString = command;
  //qDebug(command.latin1());
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
 * We have recieved some output from a thread. See if it contains a foo%.
 * @param proc the process that has new output.
 * @param buffer the output from the process
 * @param buflen the length of the buffer.
 */
void EncoderConfigImp::receivedThreadOutput(KProcess *process, char *buffer, int length){
  // Make sure we have a job to send an update too.
  Job *job = jobs[(KShellProcess*)process];
  if(!job){
    qDebug("EncoderConfigImp::receivedThreadOutput Job doesn't exists. Line: %d", __LINE__);
    return;
  }

  // Make sure the output string has a % symble in it.
  QString output = buffer;
  output = output.mid(0,length);
  int percentLocation = output.find('%');
  if(percentLocation==-1){
    qDebug("No Percent symbol found in output, not updating");
    return;
  }
  //qDebug(QString("Pre cropped: %1").arg(output).latin1());
  output = output.mid(percentLocation-encodersPercentStringLength,2);
  //qDebug(QString("Post cropped: %1").arg(output).latin1());
  bool conversionSuccessfull = false;
  int percent = output.toInt(&conversionSuccessfull);
  //qDebug(QString("number: %1").arg(percent).latin1());
  if(percent > 0 && percent < 100 && conversionSuccessfull){
    emit(updateProgress(job->id, percent));
  }
  // If it was just some random output that couldn't be converted then don't report the error.
  else if(conversionSuccessfull){
    qDebug("The Percent done (%d) is not > 0 && < 100", percent);
  }
  //else{
  //  qDebug(QString("The Percent done (%1) is not > 0 && < 100, conversion ! sucesfull").arg(output).latin1());
  //}
}

/**
 * When the thread is done encoding the file this function is called.
 * @param job the job that just finished.
 */
void EncoderConfigImp::jobDone(KProcess *process){
  if(!process)
    return;
  bool normalExit = true;
  if(process->normalExit()){
    int retrunValue = process->exitStatus();
    if(retrunValue!=0)
      qDebug("Process exited with non 0 status: %d", retrunValue);
    normalExit = false;
  }

  Job *job = jobs[(KShellProcess*)process];
  threads.remove((KShellProcess*)process);
  jobs.remove((KShellProcess*)process);

  if( QFile::exists(job->newLocation)){
    if(!saveWav->isChecked())
      QFile::remove(job->location);
    if(normalExit)
      emit(updateProgress(job->id, 100));
    if(createPlaylistCheckBox->isChecked())
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
  QString desiredFile = playlistFileFormat->text();
  QMap <QString,QString> map;
  replaceSpecialChars(desiredFile, job, false, map);
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
    KMessageBox::sorry(this, i18n("The desired playlist file could not be opened for writing to.\nPlease check your file path option."), i18n("Playlist Addition Failed"));
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

/**
 * Load up the wizard with the playlist string.  Save it if OK is hit.
 */
void EncoderConfigImp::playlistWizard(){
  fileWizard wizard(this, "Playlist File FormatWizard", true);
  wizard.playlistFormat->setText(playlistFileFormat->text());

  // Show dialog and save results if ok is pressed.
  bool okClicked = wizard.exec();
  if(okClicked){
    playlistFileFormat->setText(wizard.playlistFormat->text());
  }
}

/**
 * Load up the wizard with the encoder playlist string.  Save it if OK is hit.
 */
void EncoderConfigImp::encoderWizard(){
  fileWizard wizard(this, "Encoder File Format Wizard", true);
  wizard.playlistFormat->setText(fileFormat->text());

  // Show dialog and save results if ok is pressed.
  bool okClicked = wizard.exec();
  if(okClicked){
    fileFormat->setText(wizard.playlistFormat->text());
  }
}


#include "encoderconfigimp.moc"

// encoderconfigimp.cpp

