
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
  janusWidget = new KJanusWidget(this, name, KJanusWidget::IconList);
  setCentralWidget(janusWidget);

  QVBox * frame = janusWidget->addVBoxPage(QString("Tracks"),i18n("Tracks"), SmallIcon("cdaudio_unmount", 32));
  tracksConfig = new TracksConfigImp(frame, "TracksConfig");
  trackPage = janusWidget->pageIndex(frame);
  frame = janusWidget->addVBoxPage(QString("Cd Config"),i18n("CD Config"), SmallIcon("network", 32));
  cdConfig = new CdConfigImp(frame, "CdConfig");
  cdPage = janusWidget->pageIndex(frame);
  frame = janusWidget->addVBoxPage(QString("Ripper Config"),i18n("Ripper Config"), SmallIcon("shredder", 32));
  ripConfig = new RipConfigImp(frame, "RipConfig");
  ripPage = janusWidget->pageIndex(frame);
  frame = janusWidget->addVBoxPage(QString("Encoder Config"),i18n("Encoder Config"), SmallIcon("filter", 32));
  encoderConfig = new EncoderConfigImp(frame, "EncoderConfig");
  encoderPage = janusWidget->pageIndex(frame);
  frame = janusWidget->addVBoxPage(QString("Jobs"),i18n("Jobs"), SmallIcon("run", 32));
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
  (void)new KAction(i18n("View &CD Config"), 0, this, SLOT(viewCdConfig()), actionCollection(), "view_cd" );
  (void)new KAction(i18n("View &Ripper Config"), 0, this, SLOT(viewRipConfig()), actionCollection(), "view_rip" );
  (void)new KAction(i18n("View &Encoder Config"), 0, this, SLOT(viewEncoderConfig()), actionCollection(), "view_encoder" );
  (void)new KAction(i18n("View &Queue"), 0, this, SLOT(viewQue()), actionCollection(), "view_que" );
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
    int r = KMessageBox:: questionYesNo(this, i18n("There are unfinished jobs in the queue. Would you like to quit anyway?"), i18n("Unfinished Jobs in the queue"), i18n("&No"), i18n("&Yes"));
  // This is backwards because we want the default (first item which is YES) to be no. 
  if( r == KMessageBox::Yes )
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

void KAudioCreator::viewCdConfig(){
  janusWidget->showPage( cdPage );
}

void KAudioCreator::viewRipConfig(){
  janusWidget->showPage( ripPage );
}

void KAudioCreator::viewEncoderConfig(){
  janusWidget->showPage( encoderPage );
}

void KAudioCreator::viewQue(){
  janusWidget->showPage( quePage );
}

#include "kaudiocreator.moc"

/// kaudiocreator.cpp

