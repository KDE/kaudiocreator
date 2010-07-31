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
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "kaudiocreator.h"

#include <tqvbox.h>
#include <kiconloader.h>

#include <kmessagebox.h>
#include <kpopupmenu.h>
#include <kaction.h>
#include <kstatusbar.h>
#include <knotifydialog.h>
#include <kcombobox.h>

#include "tracksimp.h"
#include "jobqueimp.h"
#include "ripper.h"
#include "encoder.h"
#include "prefs.h"
#include "encodefileimp.h"
#include "job.h"

// Settings
#include "ripconfig.h"
#include "cdconfig.h"
#include "encoderconfigimp.h"
#include "general.h"
#include <kcmoduleloader.h>
#include <kurlrequester.h>

/**
 * Constructor. Connect all of the object and the job control.
 */
KAudioCreator::KAudioCreator( TQWidget* parent, const char* name) :
       KMainWindow(parent, name)
{
    janusWidget = new KJanusWidget(this, name, KJanusWidget::Tabbed);
    setCentralWidget(janusWidget);

    TQVBox *frame = janusWidget->addVBoxPage(i18n("&CD Tracks"), TQString::null, SmallIcon("cdaudio_unmount", 32));
    tracks = new TracksImp(frame, "Tracks");

    ripper  = new Ripper ( frame, "Rip" );
    encoder = new Encoder( frame, "Encoder" );

    frame = janusWidget->addVBoxPage( i18n("&Jobs"), TQString::null, SmallIcon( "run", 32 ) );
    jobQue = new JobQueImp( frame, "Que" );

    resize(500, 440);

    /*KAction *eject = */new KAction( i18n("&Eject CD"), 0, tracks,
                                  TQT_SLOT( eject() ), actionCollection(), "eject" );

    (void)new KAction( i18n("&Configure KAudioCreator..."), 0, this,
                       TQT_SLOT( showSettings() ), actionCollection(), "configure_kaudiocreator" );

    KAction *selectAll   = new KAction( i18n( "Select &All Tracks"), 0, tracks,
                                        TQT_SLOT( selectAllTracks()   ), actionCollection(), "select_all" ) ;
    KAction *deselectAll = new KAction( i18n( "Deselect &All Tracks"), 0, tracks,
                                        TQT_SLOT( deselectAllTracks() ), actionCollection(), "deselect_all" );
    selectAll->setEnabled( false );
    deselectAll->setEnabled( false );

    KActionMenu *actActionMenu = new KActionMenu( i18n("Rip &Selection"), "rip", actionCollection(), "rip" );
    actActionMenu->setDelayed(true); //needed for checking "all accounts"
    actActionMenu->setEnabled( false );   
    connect( actActionMenu, TQT_SIGNAL( activated() ), tracks, TQT_SLOT( startSession() ) );

    ripMenu = actActionMenu->popupMenu();
    connect( ripMenu, TQT_SIGNAL( activated(int) ), this, TQT_SLOT( slotRipSelection(int)) );
    connect( ripMenu, TQT_SIGNAL( aboutToShow() ),  this, TQT_SLOT( getRipMenu()) );

    KAction *rip = new KAction( i18n( "Rip &Selection" ), 0, tracks,
                                TQT_SLOT( startSession() ), actionCollection(), "rip_selected" );
    rip->setEnabled( false );

    connect( jobQue, TQT_SIGNAL( removeJob(int) ), this,    TQT_SLOT( updateStatus() ) );
    connect( jobQue, TQT_SIGNAL( removeJob(int) ), ripper,  TQT_SLOT( removeJob(int) ) );
    connect( jobQue, TQT_SIGNAL( removeJob(int) ), encoder, TQT_SLOT( removeJob(int)) );

    connect( ripper, TQT_SIGNAL( updateProgress(int, int) )     , jobQue,  TQT_SLOT( updateProgress(int,int) ) );
    connect( ripper, TQT_SIGNAL( addJob(Job*, const TQString &) ), jobQue,  TQT_SLOT( addJob(Job*, const TQString &)) );
    connect( ripper, TQT_SIGNAL( eject(const TQString &) )       , tracks,  TQT_SLOT( ejectDevice(const TQString &)) );
    connect( ripper, TQT_SIGNAL( encodeWav(Job *) )             , encoder, TQT_SLOT( encodeWav(Job *)) );
    connect( ripper, TQT_SIGNAL( jobsChanged() )                , this,    TQT_SLOT( updateStatus() ) );   

    connect( encoder, TQT_SIGNAL( updateProgress(int, int) )   , jobQue,  TQT_SLOT( updateProgress(int,int)) );
    connect( encoder, TQT_SIGNAL( addJob(Job*, const TQString&)), jobQue,  TQT_SLOT( addJob(Job*, const TQString &)) );
    connect( encoder, TQT_SIGNAL( jobsChanged() )              , this,    TQT_SLOT( updateStatus() ) );

    connect( tracks, TQT_SIGNAL( hasCD(bool) )    , this,          TQT_SLOT( hasCD(bool) ) );
    connect( tracks, TQT_SIGNAL( ripTrack(Job *) ), ripper,        TQT_SLOT( ripTrack(Job *)) );
    connect( tracks, TQT_SIGNAL( hasTracks(bool) ), rip,           TQT_SLOT( setEnabled(bool)) );
    connect( tracks, TQT_SIGNAL( hasTracks(bool) ), actActionMenu, TQT_SLOT( setEnabled(bool)) );
    connect( tracks, TQT_SIGNAL( hasTracks(bool) ), deselectAll,   TQT_SLOT( setEnabled(bool)) );
    connect( tracks, TQT_SIGNAL( hasTracks(bool) ), selectAll,     TQT_SLOT( setEnabled(bool)) );

    (void)new KAction(i18n("Remove &Completed Jobs"), 0, jobQue,
          TQT_SLOT(clearDoneJobs()), actionCollection(), "clear_done_jobs" );

    KAction *edit = new KAction(i18n("&Edit Album..."), 0, tracks,
          TQT_SLOT(editInformation()), actionCollection(), "edit_cd");
    connect(tracks, TQT_SIGNAL(hasCD(bool)), edit, TQT_SLOT(setEnabled(bool)));
    edit->setEnabled( false );

    (void)new KAction(i18n("Encode &File..."), 0, this,
          TQT_SLOT(encodeFile()), actionCollection(), "encode_file");

    KAction *cddb = new KAction(i18n("&CDDB Lookup"), 0, tracks,
          TQT_SLOT(performCDDB()), actionCollection(), "cddb_now");
    connect(tracks, TQT_SIGNAL(hasCD(bool)), cddb, TQT_SLOT(setEnabled(bool)));
    cddb->setEnabled( false );

    KStdAction::configureNotifications(this, TQT_SLOT(configureNotifications()),
          actionCollection());
    KStdAction::quit( this, TQT_SLOT(close()), actionCollection(), "quit" );

    // Init statusbar
    statusBar()->insertItem(i18n("No Audio CD detected"), 0 );
    hasCD(tracks->hasCD());

    setupGUI();
}

void KAudioCreator::setDevice( const TQString &device )
{
    tracks->deviceCombo->setCurrentText( device );
}

void KAudioCreator::slotRipSelection(int selection){
    tracks->startSession( selection );
}

void KAudioCreator::getRipMenu(){
    ripMenu->clear();

    int i=0;
    TQString currentGroup = TQString("Encoder_%1").arg(i);
    while(EncoderPrefs::hasPrefs(currentGroup)){
        ripMenu->insertItem(EncoderPrefs::prefs(currentGroup)->encoderName(), i);
        currentGroup = TQString("Encoder_%1").arg(++i);
    }
}

/**
 * Changes the status bar to let the user no if a cd was not detected or inserted.
 */
void KAudioCreator::hasCD(bool cd){
    if(cd)
        statusBar()->changeItem(i18n("CD Inserted"), 0 );
    else
        statusBar()->changeItem(i18n("No Audio CD detected"), 0 );
}

void KAudioCreator::updateStatus() {
    TQString status = i18n("Idle.");
    TQString rippingStatus;
    TQString encodingStatus;
    int activeRippingJobs = ripper->activeJobCount();
    int pendingRippingJobs = ripper->pendingJobCount();
    int activeEncodingJobs = encoder->activeJobCount();
    int pendingEncodingJobs = encoder->pendingJobCount();

    if ( activeRippingJobs ) {
        rippingStatus = i18n("Ripping (%1 active, %2 queued)").arg( activeRippingJobs ).arg( pendingRippingJobs );
        status = rippingStatus;
    }
    if ( activeEncodingJobs ) {
        encodingStatus = i18n("Encoding (%1 active, %2 queued)").arg( activeEncodingJobs ).arg( pendingEncodingJobs );

        if ( activeRippingJobs ) {
            status.append(" : ");
            status.append( encodingStatus );
        }
        else {
            status = encodingStatus;
        }
    }

    statusBar()->changeItem( status, 0 );
}

/**
 * Ask the user if they really want to quit if there are open jobs.
 */
bool KAudioCreator::queryClose() {
    if(jobQue->numberOfJobsNotFinished() > 0 &&
        (KMessageBox::warningContinueCancel(this, i18n("There are unfinished jobs in the queue. Would you like to quit anyway?"), i18n("Unfinished Jobs in Queue"),KStdGuiItem::quit())
              == KMessageBox::Cancel ))
        return false;
    return true;
}

void KAudioCreator::configureNotifications() {
    KNotifyDialog *dialog = new KNotifyDialog(this, "KNotifyDialog", false);
    dialog->show();
}

void KAudioCreator::encodeFile(){
    EncodeFileImp *file = new EncodeFileImp(this, "EncodeFile");
    connect(file, TQT_SIGNAL(startJob(Job*)),encoder, TQT_SLOT(encodeWav(Job*)));
    file->show();
}

/**
 * Show Settings dialog.
 */
void KAudioCreator::showSettings(){
    if(KConfigDialog::showDialog("settings"))
        return;

    SettingsDialog *dialog = new SettingsDialog(this, "settings", Prefs::self());
    connect(dialog, TQT_SIGNAL(settingsChanged()), ripper, TQT_SLOT(loadSettings()));
    connect(dialog, TQT_SIGNAL(settingsChanged()), encoder, TQT_SLOT(loadSettings()));
    connect(dialog, TQT_SIGNAL(settingsChanged()), tracks, TQT_SLOT(loadSettings()));
    connect(dialog->encoderConfigImp, TQT_SIGNAL(encoderUpdated()), encoder, TQT_SLOT(loadSettings()));
    dialog->show();
}

SettingsDialog::SettingsDialog(TQWidget *parent, const char *name,KConfigSkeleton *config)
 : KConfigDialog(parent, name, config),
 cddb(0), cddbChanged(false)
{
    addPage(new General(0, "General"), i18n("General"), "package_settings",
          i18n("General Configuration"));
    addPage(new CdConfig(0, "CD"), i18n("CD"), "package_system",
          i18n("CD Configuration"));

    // Because WE don't segfault on our users...
    KService::Ptr libkcddb = KService::serviceByDesktopName("libkcddb");
    if (libkcddb && libkcddb->isValid())
    {
        cddb = KCModuleLoader::loadModule(TQString("libkcddb"), KCModuleLoader::Inline);
        if (cddb)
        {
            cddb->load();
            addPage(cddb, i18n("CDDB"), "cdaudio_mount", i18n("CDDB Configuration"), false);
            connect(cddb, TQT_SIGNAL(changed(bool)), this, TQT_SLOT(slotCddbChanged(bool)));
        }
    }
    RipConfig *rip = new RipConfig(0, "Ripper");
    rip->kcfg_tempDir->setMode(KFile::Directory);
    addPage(rip, i18n("Ripper"), "gear", i18n("Ripper Configuration") );

    encoderConfigImp = new EncoderConfigImp(0, "Encoder");
    addPage(encoderConfigImp, i18n("Encoder"), "filter", i18n("Encoder Configuration") );
}

void SettingsDialog::updateSettings()
{
    if (cddb)
        cddb->save();
}

void SettingsDialog::updateWidgets()
{
    if (cddb)
        cddb->load();
    cddbChanged = false;
}

void SettingsDialog::updateWidgetsDefault()
{
    if (cddb)
        cddb->defaults();
}

bool SettingsDialog::hasChanged()
{
    return cddbChanged;
}

bool SettingsDialog::isDefault()
{
    if (cddb)
        return false;
    return true;
}

void SettingsDialog::slotCddbChanged(bool changed)
{
    cddbChanged = changed;
    updateButtons();
}

#include "kaudiocreator.moc"

