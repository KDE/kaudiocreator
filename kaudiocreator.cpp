#include "kaudiocreator.h"

#include <qvbox.h>
#include <kiconloader.h>

#include <kmessagebox.h>

#include <kaction.h>
#include <kedittoolbar.h>

#include "tracksconfigimp.h"
#include "cdconfigimp.h"
#include "ripper.h"
#include "encoder.h"
#include "jobqueimp.h"

#include "options.h"

/**
 * Constructor.  Connect all of the object and the job control.
 */
KAudioCreator::KAudioCreator( QWidget* parent, const char* name) : KMainWindow(parent, name){
  janusWidget = new KJanusWidget(this, name, KJanusWidget::Tabbed);
  setCentralWidget(janusWidget);

  QVBox * frame = janusWidget->addVBoxPage(i18n("&CD Tracks"),QString::null, SmallIcon("cdaudio_unmount", 32));
  tracksConfig = new TracksConfigImp(frame, "Tracks");
  
  cdConfig = new CdConfigImp(frame, "Cd");
  ripper = new Ripper(frame, "Rip");
  encoder = new Encoder(frame, "Encoder");

  QFrame *f = janusWidget->addPage(i18n("&Jobs"), QString::null, SmallIcon("run", 32));
  jobQue = new JobQueImp(f, "Que");

  connect(jobQue, SIGNAL(removeJob(int)), ripper, SLOT(removeJob(int)));
  connect(ripper, SIGNAL(updateProgress(int, int)), jobQue, SLOT(updateProgress(int,int)));
  connect(ripper, SIGNAL(addJob(Job*, QString)), jobQue, SLOT(addJob(Job*, QString)));

  connect(jobQue, SIGNAL(removeJob(int)), encoder, SLOT(removeJob(int)));
  connect(encoder, SIGNAL(updateProgress(int, int)), jobQue, SLOT(updateProgress(int,int)));
  connect(encoder, SIGNAL(addJob(Job*, QString)), jobQue, SLOT(addJob(Job*, QString)));
  connect(tracksConfig, SIGNAL(refreshCd()), cdConfig, SLOT(timerDone()));

  connect(cdConfig, SIGNAL(newAlbum(QString, QString, int, QString)), tracksConfig, SLOT(newAlbum(QString, QString, int, QString)));
  connect(cdConfig, SIGNAL(newSong(int, QString, int)), tracksConfig, SLOT(newSong(int, QString, int)));
  connect(tracksConfig, SIGNAL(ripTrack(Job *)), ripper, SLOT(ripTrack(Job *)));

  connect(ripper, SIGNAL(encodeWav(Job *)), encoder, SLOT(encodeWav(Job *)));
  connect(cdConfig, SIGNAL(ripAlbum()), tracksConfig, SLOT(ripWholeAlbum()));

  resize(500, 440);

  (void)new KAction(i18n("&Configure KAudioCreator..."), 0, this, SLOT(showOptions()), actionCollection(), "configure_kaudiocreator" );
  (void)new KAction(i18n("Rip &Selected Tracks"), 0, tracksConfig, SLOT(startSession()), actionCollection(), "rip" );
  (void)new KAction(i18n("Remove &Completed Jobs"), 0, jobQue, SLOT(clearDoneJobs()), actionCollection(), "clear_done_jobs" );
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
  if(jobQue->numberOfJobsNotFinished() > 0 && 
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
  connect(optionsDialog, SIGNAL(readNewOptions()), ripper, SLOT(loadSettings()));
  connect(optionsDialog, SIGNAL(readNewOptions()), encoder, SLOT(loadSettings()));
  connect(optionsDialog, SIGNAL(readNewOptions()), jobQue, SLOT(loadSettings()));
}

/**
 * Close KAudioCreator Options.
 */
void KAudioCreator::closeOptions(){
  delete optionsDialog;
}

#include "kaudiocreator.moc"

