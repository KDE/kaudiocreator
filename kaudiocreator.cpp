
#include "kaudiocreator.h"
#include <kjanuswidget.h>
#include <qframe.h>
#include <qvbox.h>
#include <qstring.h>
#include <kiconloader.h>

#include <qglobal.h>
#if QT_VERSION < 300
#include <kapp.h>
#else
#include <kapplication.h>
#endif

#include <kmessagebox.h>

#include <klocale.h>
#include <kaction.h>
#include <kstdaction.h>
#include <kedittoolbar.h>
#include <kconfig.h>
#include <kdialogbase.h>

#include "tracksconfigimp.h"
#include "cdconfigimp.h"
#include "ripconfigimp.h"
#include "encoderconfigimp.h"
#include "queconfigimp.h"
#include "job.h"

/**
 * Constructor.  Connect all of the object and the job control.
 */
KAudioCreator::KAudioCreator( QWidget* parent, const char* name) : KMainWindow(parent, name){
  janusWidget = new KJanusWidget(this, name, KJanusWidget::Tabbed);
  setCentralWidget(janusWidget);

  options = new KDialogBase(KDialogBase::IconList, i18n("Options"), 0x00000004,KDialogBase::Ok,this, "Options Dialog");
	 
  QVBox * frame = janusWidget->addVBoxPage(i18n("CD Tracks"),i18n("CD Tracks"), SmallIcon("cdaudio_unmount", 32));
  tracksConfig = new TracksConfigImp(frame, "TracksConfig");
  trackPage = janusWidget->pageIndex(frame);
  
  frame = options->addVBoxPage(i18n("Cd Config"),i18n("CD Config"), SmallIcon("network", 32));
  cdConfig = new CdConfigImp(frame, "CdConfig");

  frame = options->addVBoxPage(i18n("Ripper Config"),i18n("Ripper Config"), SmallIcon("shredder", 32));
  ripConfig = new RipConfigImp(frame, "RipConfig");
  
  frame = options->addVBoxPage(i18n("Encoder Config"),i18n("Encoder Config"), SmallIcon("filter", 32));
  encoderConfig = new EncoderConfigImp(frame, "EncoderConfig");
  
  frame = janusWidget->addVBoxPage(i18n("Jobs"),i18n("Jobs"), SmallIcon("run", 32));
  queConfig = new QueConfigImp(frame, "QueConfig");
  quePage = janusWidget->pageIndex(frame);

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

  (void)new KAction(i18n("View &Tracks"), 0, this, SLOT(viewTracks()), actionCollection(), "view_tracks" );
  (void)new KAction(i18n("&Configure KAudioCreator"), 0, this, SLOT(showOptions()), actionCollection(), "configure_kaudiocreator" );
  (void)new KAction(i18n("View &Jobs"), 0, this, SLOT(viewQue()), actionCollection(), "view_que" );
  (void)new KAction(i18n("Rip &Selected Tracks"), 0, tracksConfig, SLOT(startSession()), actionCollection(), "rip" );
  (void)new KAction(i18n("Remove &Completed Jobs"), 0, queConfig, SLOT(clearDoneJobs()), actionCollection(), "clear_done_jobs" );
  (void)new KAction(i18n("&Refresh CD List"), 0, cdConfig, SLOT(timerDone()), actionCollection(), "update_cd" );
  (void)new KAction(i18n("&Edit Album"), 0, tracksConfig, SLOT(editInformation()), actionCollection(), "edit_cd");
  (void)new KAction(i18n("&Perform CDDB Lookup"), 0, cdConfig, SLOT(cddbNow()), actionCollection(), "cddb_now");
  KStdAction::configureToolbars(this, SLOT(configuretoolbars() ), actionCollection(), "configuretoolbars");

  KStdAction::quit( this, SLOT(quit()), actionCollection(), "quit" );
 
  createGUI("kaudiocreatorui.rc");
}

/**
 * Deconstructor save toolbar settings.
 */
KAudioCreator::~KAudioCreator(){
  KConfig &config = *KGlobal::config();
  toolBar("Main ToolBar")->saveSettings(&config, "Main Toolbar");
}

/**
 * If there are jobs in the que promt the user before quiting.
 */
void KAudioCreator::quit(){
  this->close(true);
}

/**
 * Call quit which will ask the user if they really want to quit.
 */
void KAudioCreator::closeEvent(QCloseEvent *e) {
  if(queConfig->numberOfJobsNotFinished() > 0){
    int r = KMessageBox:: questionYesNo(this, i18n("There are unfinished jobs in the queue. Would you like to quit anyway?"), i18n("Unfinished Jobs in the queue"));
  if( r == KMessageBox::No )
    return;
  }
  QWidget::closeEvent(e);
}

/**
 * Allow for the toolbars to be minipulated.
 */
void KAudioCreator::configuretoolbars(){
  KEditToolbar dlg(actionCollection(), "kaudiocreatorui.rc");
  if(dlg.exec())
    createGUI("kaudiocreatorui.rc");
}

void KAudioCreator::viewTracks(){
  janusWidget->showPage( trackPage );
}

void KAudioCreator::showOptions(){
  options->show();
}

void KAudioCreator::viewQue(){
  janusWidget->showPage( quePage );
}

#include "kaudiocreator.moc"

/// kaudiocreator.cpp

