#include "kaudiocreator.h"

#include <qvbox.h>
#include <kiconloader.h>

#include <kmessagebox.h>

#include <kaction.h>
#include <kedittoolbar.h>

#include "tracksimp.h"
#include "jobqueimp.h"
#include "ripper.h"
#include "encoder.h"

// Settings
#include <kautoconfigdialog.h>
#include "ripconfig.h"
#include "cdconfig.h"
#include "encoderconfigimp.h"
#include "general.h"
#include <kcmoduleloader.h>

/**
 * Constructor.  Connect all of the object and the job control.
 */
KAudioCreator::KAudioCreator( QWidget* parent, const char* name) : KMainWindow(parent, name){
  janusWidget = new KJanusWidget(this, name, KJanusWidget::Tabbed);
  setCentralWidget(janusWidget);

  QVBox * frame = janusWidget->addVBoxPage(i18n("&CD Tracks"),QString::null, SmallIcon("cdaudio_unmount", 32));
  tracks = new TracksImp(frame, "Tracks");

  ripper = new Ripper(frame, "Rip");
  encoder = new Encoder(frame, "Encoder");

  frame = janusWidget->addVBoxPage(i18n("&Jobs"), QString::null, SmallIcon("run", 32));
  jobQue = new JobQueImp(frame, "Que");

  connect(jobQue, SIGNAL(removeJob(int)), ripper, SLOT(removeJob(int)));
  connect(ripper, SIGNAL(updateProgress(int, int)), jobQue, SLOT(updateProgress(int,int)));
  connect(ripper, SIGNAL(addJob(Job*, QString)), jobQue, SLOT(addJob(Job*, QString)));

  connect(jobQue, SIGNAL(removeJob(int)), encoder, SLOT(removeJob(int)));
  connect(encoder, SIGNAL(updateProgress(int, int)), jobQue, SLOT(updateProgress(int,int)));
  connect(encoder, SIGNAL(addJob(Job*, QString)), jobQue, SLOT(addJob(Job*, QString)));

  connect(tracks, SIGNAL(ripTrack(Job *)), ripper, SLOT(ripTrack(Job *)));
  connect(ripper, SIGNAL(eject()), tracks, SLOT(eject()));


  connect(ripper, SIGNAL(encodeWav(Job *)), encoder, SLOT(encodeWav(Job *)));

  resize(500, 440);

  (void)new KAction(i18n("&Eject CD"), 0, tracks, SLOT(eject()), actionCollection(), "eject" );
  (void)new KAction(i18n("&Configure KAudioCreator..."), 0, this, SLOT(showSettings()), actionCollection(), "configure_kaudiocreator" );
  (void)new KAction(i18n("Rip &Selected Tracks"), 0, tracks, SLOT(startSession()), actionCollection(), "rip" );
  (void)new KAction(i18n("Remove &Completed Jobs"), 0, jobQue, SLOT(clearDoneJobs()), actionCollection(), "clear_done_jobs" );
  (void)new KAction(i18n("&Edit Album"), 0, tracks, SLOT(editInformation()), actionCollection(), "edit_cd");
  (void)new KAction(i18n("&Perform CDDB Lookup"), 0, tracks, SLOT(performCDDB()), actionCollection(), "cddb_now");
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
 * Show Settings dialog.
 */
void KAudioCreator::showSettings(){
  if(KAutoConfigDialog::showDialog("settings"))
    return;

  KAutoConfigDialog *dialog = new KAutoConfigDialog(this, "settings");
  dialog->addPage(new General(0, "General"), i18n("General"), "General", "package_settings", i18n("General Configureation"));
  dialog->addPage(new CdConfig(0, "CD"), i18n("CD"), "CD", "package_system", i18n("CD Configuration"));

  // Because WE don't segfault on our users...
  KService::Ptr libkcddb = KService::serviceByDesktopName("libkcddb");
  if (libkcddb->isValid())
  {
    KCModuleInfo info(libkcddb->desktopEntryPath(), "settings");
    if (info.service()->isValid())
    {
      KCModule *m = KCModuleLoader::loadModule(info);
      if (m)
      {
        m->load();
        dialog->addPage(m, i18n("CDDB"), "Game", "cdaudio_unmount", i18n("CDDB Configuration"), false);
        connect(dialog, SIGNAL(okClicked()), m, SLOT(save()));
        connect(dialog, SIGNAL(applyClicked()), m, SLOT(save()));
        connect(dialog, SIGNAL(defaultClicked()), m, SLOT(defaults()));
      }
    }
  }

  dialog->addPage(new RipConfig(0, "Ripper"), i18n("Ripper"), "Ripper", "shredder", i18n("Ripper Configuration") );
  EncoderConfigImp *encoderConfigImp = new EncoderConfigImp(0, "Encoder");
  dialog->addPage(encoderConfigImp, i18n("Encoder"), "Encoder", "filter", i18n("Encoder Configuration") );

  connect(encoderConfigImp, SIGNAL(encoderUpdated()), encoder, SLOT(loadSettings()));

  connect(dialog, SIGNAL(settingsChanged()), ripper, SLOT(loadSettings()));
  connect(dialog, SIGNAL(settingsChanged()), encoder, SLOT(loadSettings()));
  connect(dialog, SIGNAL(settingsChanged()), jobQue, SLOT(loadSettings()));
  connect(dialog, SIGNAL(settingsChanged()), tracks, SLOT(loadSettings()));
  dialog->show();
}

#include "kaudiocreator.moc"

