#include "kaudiocreator.h"

#include <kjanuswidget.h>
#include <qvbox.h>
#include <kiconloader.h>

#include <kmessagebox.h>

#include <klocale.h>
#include <kaction.h>
#include <kstdaction.h>
#include <kedittoolbar.h>

#include "tracksconfigimp.h"
#include "cdconfigimp.h"
#include "ripconfigimp.h"
#include "encoderconfigimp.h"
#include "queconfigimp.h"
#include "job.h"

#include "options.h"

/**
 * Constructor.  Connect all of the object and the job control.
 */
KAudioCreator::KAudioCreator( QWidget* parent, const char* name) : KMainWindow(parent, name){
  janusWidget = new KJanusWidget(this, name, KJanusWidget::Tabbed);
  setCentralWidget(janusWidget);

  QVBox * frame = janusWidget->addVBoxPage(i18n("&CD Tracks"),QString::null, SmallIcon("cdaudio_unmount", 32));
  tracksConfig = new TracksConfigImp(frame, "Tracks");
  
  cdConfig = new CdConfigImp(frame, "CdConfig");
  ripConfig = new RipConfigImp(frame, "RipConfig");
  encoderConfig = new EncoderConfigImp(frame, "EncoderConfig");

  frame = janusWidget->addVBoxPage(i18n("&Jobs"), QString::null, SmallIcon("run", 32));
  queConfig = new QueConfigImp(frame, "Que");

  connect(queConfig, SIGNAL(removeJob(int)), ripConfig, SLOT(removeJob(int)));
  connect(ripConfig, SIGNAL(updateProgress(int, int)), queConfig, SLOT(updateProgress(int,int)));
  connect(ripConfig, SIGNAL(addJob(Job*, QString)), queConfig, SLOT(addJob(Job*, QString)));

  connect(queConfig, SIGNAL(removeJob(int)), encoderConfig, SLOT(removeJob(int)));
  connect(encoderConfig, SIGNAL(updateProgress(int, int)), queConfig, SLOT(updateProgress(int,int)));
  connect(encoderConfig, SIGNAL(addJob(Job*, QString)), queConfig, SLOT(addJob(Job*, QString)));
  connect(tracksConfig, SIGNAL(refreshCd()), cdConfig, SLOT(timerDone()));

  connect(cdConfig, SIGNAL(newAlbum(QString, QString, int, QString)), tracksConfig, SLOT(newAlbum(QString, QString, int, QString)));
  connect(cdConfig, SIGNAL(newSong(int, QString, int)), tracksConfig, SLOT(newSong(int, QString, int)));
  connect(tracksConfig, SIGNAL(ripTrack(Job *)), ripConfig, SLOT(ripTrack(Job *)));

  connect(ripConfig, SIGNAL(encodeWav(Job *)), encoderConfig, SLOT(encodeWav(Job *)));
  connect(cdConfig, SIGNAL(ripAlbum()), tracksConfig, SLOT(ripWholeAlbum()));

  resize(500, 440);

  (void)new KAction(i18n("&Configure KAudioCreator..."), 0, this, SLOT(showOptions()), actionCollection(), "configure_kaudiocreator" );
  (void)new KAction(i18n("Rip &Selected Tracks"), 0, tracksConfig, SLOT(startSession()), actionCollection(), "rip" );
  (void)new KAction(i18n("Remove &Completed Jobs"), 0, queConfig, SLOT(clearDoneJobs()), actionCollection(), "clear_done_jobs" );
  (void)new KAction(i18n("&Refresh CD List"), 0, cdConfig, SLOT(timerDone()), actionCollection(), "update_cd" );
  (void)new KAction(i18n("&Edit Album"), 0, tracksConfig, SLOT(editInformation()), actionCollection(), "edit_cd");
  (void)new KAction(i18n("&Perform CDDB Lookup"), 0, cdConfig, SLOT(cddbNow()), actionCollection(), "cddb_now");
  KStdAction::configureToolbars(this, SLOT(configuretoolbars() ), actionCollection(), "configuretoolbars");
  setStandardToolBarMenuEnabled(true);

  KStdAction::quit( this, SLOT(close()), actionCollection(), "quit" );

  createGUI("kaudiocreatorui.rc");
  setAutoSaveSettings( "Main Window" );
}

/**
 * Ask the user if they really want to quit if there are open jobs.
 */
bool KAudioCreator::queryClose() {
  if(queConfig->numberOfJobsNotFinished() > 0 && 
    (KMessageBox::questionYesNo(this, i18n("There are unfinished jobs in the queue. Would you like to quit anyway?"), i18n("Unfinished Jobs in the queue"))
      == KMessageBox::No ))
    return false;
  return true;
}

/**
 * Allow for the toolbars to be minipulated.
 */
void KAudioCreator::configuretoolbars(){
  saveMainWindowSettings(KGlobal::config(), "Main Window");
  KEditToolbar dlg(actionCollection(), "kaudiocreatorui.rc");
  connect(&dlg, SIGNAL(newToolbarConfig()), SLOT(saveToolbarConfig()));
  dlg.exec();
}

/**
 * Save new toolbarconfig.
 */
void KAudioCreator::saveToolbarConfig(){
  createGUI("kaudiocreatorui.rc");
  applyMainWindowSettings(KGlobal::config(), "Main Window");
}

/**
 * Show KAudioCreator Options.
 */
void KAudioCreator::showOptions(){
  optionsDialog = new Options(this, "Options");
  connect(optionsDialog, SIGNAL(closeOptions()), this, SLOT(closeOptions()));

  connect(optionsDialog, SIGNAL(readNewOptions()), cdConfig, SLOT(loadSettings()));
  connect(optionsDialog, SIGNAL(readNewOptions()), ripConfig, SLOT(loadSettings()));
  connect(optionsDialog, SIGNAL(readNewOptions()), encoderConfig, SLOT(loadSettings()));
  connect(optionsDialog, SIGNAL(readNewOptions()), queConfig, SLOT(loadSettings()));
}

/**
 * Close KAudioCreator Options.
 */
void KAudioCreator::closeOptions(){
  delete optionsDialog;
}

#include "kaudiocreator.moc"

