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
 * the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

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
#include "prefs.h"

// Settings
#include "ripconfig.h"
#include "cdconfig.h"
#include "encoderconfigimp.h"
#include "general.h"
#include <kcmoduleloader.h>
#include <kkeydialog.h>
#include <kurlrequester.h>

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
  connect(ripper, SIGNAL(addJob(Job*, const QString &)), jobQue, SLOT(addJob(Job*, const QString &)));

  connect(jobQue, SIGNAL(removeJob(int)), encoder, SLOT(removeJob(int)));
  connect(encoder, SIGNAL(updateProgress(int, int)), jobQue, SLOT(updateProgress(int,int)));
  connect(encoder, SIGNAL(addJob(Job*, const QString &)), jobQue, SLOT(addJob(Job*, const QString &)));

  connect(tracks, SIGNAL(ripTrack(Job *)), ripper, SLOT(ripTrack(Job *)));
  connect(ripper, SIGNAL(eject()), tracks, SLOT(eject()));


  connect(ripper, SIGNAL(encodeWav(Job *)), encoder, SLOT(encodeWav(Job *)));

  resize(500, 440);

  KAction *eject = new KAction(i18n("&Eject CD"), 0, tracks, SLOT(eject()), actionCollection(), "eject" );
  connect(tracks, SIGNAL(hasTracks(bool)), eject, SLOT(setEnabled(bool)));
  
  (void)new KAction(i18n("&Configure KAudioCreator..."), 0, this, SLOT(showSettings()), actionCollection(), "configure_kaudiocreator" );
  
  KAction *selectAll = new KAction(i18n("Select &All Tracks"), 0, tracks, SLOT(selectAllTracks()), actionCollection(), "select_all" );
  connect(tracks, SIGNAL(hasTracks(bool)), selectAll, SLOT(setEnabled(bool)));
  
  KAction *deselectAll = new KAction(i18n("Deselect &All Tracks"), 0, tracks, SLOT(deselectAllTracks()), actionCollection(), "deselect_all" );
  connect(tracks, SIGNAL(hasTracks(bool)), deselectAll, SLOT(setEnabled(bool)));
  
  KAction *rip = new KAction(i18n("Rip &Selection"), 0, tracks, SLOT(startSession()), actionCollection(), "rip" );
  connect(tracks, SIGNAL(hasTracks(bool)), rip, SLOT(setEnabled(bool)));
  
  (void)new KAction(i18n("Remove &Completed Jobs"), 0, jobQue, SLOT(clearDoneJobs()), actionCollection(), "clear_done_jobs" );
  
  KAction *edit = new KAction(i18n("&Edit Album"), 0, tracks, SLOT(editInformation()), actionCollection(), "edit_cd");
  connect(tracks, SIGNAL(hasCD(bool)), edit, SLOT(setEnabled(bool)));
  
  KAction *cddb = new KAction(i18n("&CDDB Lookup"), 0, tracks, SLOT(performCDDB()), actionCollection(), "cddb_now");
  connect(tracks, SIGNAL(hasCD(bool)), cddb, SLOT(setEnabled(bool)));
  
  KStdAction::configureToolbars(this, SLOT(configuretoolbars() ), actionCollection(), "configuretoolbars");
  setStandardToolBarMenuEnabled(true);

  KStdAction::quit( this, SLOT(close()), actionCollection(), "quit" );
  KStdAction::keyBindings( this, SLOT( slotConfigureKeys() ), actionCollection() );

  createGUI("kaudiocreatorui.rc");
  setAutoSaveSettings( "Main Window" );
}

void KAudioCreator::slotConfigureKeys()
{
  KKeyDialog::configure( actionCollection(), this );
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
  if(KConfigDialog::showDialog("settings"))
    return;

  SettingsDialog *dialog = new SettingsDialog(this, "settings", Prefs::self());
  connect(dialog, SIGNAL(settingsChanged()), ripper, SLOT(loadSettings()));
  connect(dialog, SIGNAL(settingsChanged()), encoder, SLOT(loadSettings()));
  connect(dialog, SIGNAL(settingsChanged()), jobQue, SLOT(loadSettings()));
  connect(dialog, SIGNAL(settingsChanged()), tracks, SLOT(loadSettings()));
  connect(dialog->encoderConfigImp, SIGNAL(encoderUpdated()), encoder, SLOT(loadSettings()));
  dialog->show();
}

SettingsDialog::SettingsDialog(QWidget *parent, const char *name,KConfigSkeleton *config)
 : KConfigDialog(parent, name, config),
 cddb(0), cddbChanged(false)
{
  addPage(new General(0, "General"), i18n("General"), "package_settings", i18n("General Configuration"));
  addPage(new CdConfig(0, "CD"), i18n("CD"), "package_system", i18n("CD Configuration"));

  // Because WE don't segfault on our users...
  KService::Ptr libkcddb = KService::serviceByDesktopName("libkcddb");
  if (libkcddb && libkcddb->isValid())
  {
    KCModuleInfo info(libkcddb->desktopEntryPath());
    if (info.service()->isValid())
    {
      cddb = KCModuleLoader::loadModule(info);
      if (cddb)
      {
        cddb->load();
        addPage(cddb, i18n("CDDB"), "cdaudio_unmount", i18n("CDDB Configuration"), false);
	connect(cddb, SIGNAL(changed(bool)), this, SLOT(slotCddbChanged(bool)));
      }
    }
  }
  RipConfig *rip = new RipConfig(0, "Ripper");
  rip->kcfg_tempDir->setMode(KFile::Directory);
  addPage(rip, i18n("Ripper"), "shredder", i18n("Ripper Configuration") );
  
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

