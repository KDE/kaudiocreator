/**
 * Copyright (C) 2003-2005 Benjamin C Meyer (ben at meyerhome dot net)
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

#include <QTimer>

#include <kicon.h>
#include <kmessagebox.h>
#include <kmenu.h>
#include <kaction.h>
#include <kstatusbar.h>
#include <kcombobox.h>
#include <knotifyconfigwidget.h>
#include <kstandarddirs.h>
#include <kglobal.h>

#include "tracksimp.h"
#include "jobqueimp.h"
#include "encodefileimp.h"
#include "ripper.h"
#include "encoder.h"
#include "prefs.h"
#include "job.h"
#include "kaudiocreator.h"

// Settings
#include <kcmoduleloader.h>
#include <kurlrequester.h>
#include <kstandardaction.h>
#include <kactionmenu.h>
#include <kactioncollection.h>

#include "encoderconfigimp.h"
#include "general.h"

/**
 * Constructor. Connect all of the object and the job control.
 */
KAudioCreator::KAudioCreator( QWidget *parent) : KXmlGuiWindow(parent), driveLabel(0), ripLabel(0)
{
	pageWidget = new KPageWidget(this);
	pageWidget->setFaceType(KPageView::Tabbed);
	setCentralWidget(pageWidget);

	tracks = new TracksImp();
	connect(tracks, SIGNAL(driveStatusChanged(AudioCD::DriveStatus)), this, SLOT(setDriveStatus(AudioCD::DriveStatus)));

	trackPage = new KPageWidgetItem(tracks, i18n("&CD Tracks"));
	trackPage->setIcon(KIcon("media-optical-audio"));
	pageWidget->addPage(trackPage);

	checkSettings();

	ripper = new Ripper(this);
	encoder = new Encoder(this);
	jobQue = new JobQueImp(0);

	jobPage = new KPageWidgetItem(jobQue, i18n("&Jobs"));
	jobPage->setIcon(KIcon("system-run"));
	pageWidget->addPage(jobPage);

	connect(tracks, SIGNAL(sessionStarted()), this, SLOT(showJobPage()));

	connect(jobQue, SIGNAL(removeJob(int)), ripper, SLOT(removeJob(int)));
	connect(ripper, SIGNAL(updateProgress(int, int)), jobQue,
		  SLOT(updateProgress(int,int)));
	connect(ripper, SIGNAL(addJob(Job*, const QString &)), jobQue,
		  SLOT(addJob(Job*, const QString &)));

	connect(jobQue, SIGNAL(removeJob(int)), encoder, SLOT(removeJob(int)));
	connect(encoder, SIGNAL(updateProgress(int, int)), jobQue,
		  SLOT(updateProgress(int,int)));
	connect(encoder, SIGNAL(addJob(Job*, const QString &)), jobQue,
		  SLOT(addJob(Job*, const QString &)));

	connect(tracks, SIGNAL(ripTrack(Job *)), ripper, SLOT(ripTrack(Job *)));
	connect(ripper, SIGNAL(eject(const QString &)), tracks,
	  SLOT(ejectDevice(const QString &)));

	connect(ripper, SIGNAL(encodeWav(Job *)), encoder, SLOT(encodeWav(Job *)));

	connect( ripper, SIGNAL( jobsChanged() ), this, SLOT( setRipStatus() ) );
	connect( encoder, SIGNAL( jobsChanged() ), this, SLOT( setRipStatus() ) );
	connect( jobQue, SIGNAL( removeJob( int ) ), this, SLOT( setRipStatus() ) );

	QAction *eject = actionCollection()->addAction("eject");
	eject->setText(i18n("&Eject CD"));
	connect(eject, SIGNAL(triggered(bool) ), tracks, SLOT(eject()));

	QAction *action = actionCollection()->addAction("configure_kaudiocreator");
        action->setText(i18n("&Configure KAudioCreator..."));
	connect(action, SIGNAL(triggered(bool) ), SLOT(showSettings()));

	QAction *selectAll = actionCollection()->addAction("select_all");
	selectAll->setText(i18n("Select &All Tracks"));
	connect(selectAll, SIGNAL(triggered(bool) ), tracks, SLOT(selectAllTracks()));
	connect(this, SIGNAL(hasAudioCd(bool)), selectAll, SLOT(setEnabled(bool)));

	QAction *deselectAll = actionCollection()->addAction("deselect_all");
	deselectAll->setText(i18n("Deselect &All Tracks"));
	connect(deselectAll, SIGNAL(triggered(bool) ), tracks, SLOT(deselectAllTracks()));
	connect(this, SIGNAL(hasAudioCd(bool)), deselectAll, SLOT(setEnabled(bool)));
	selectAll->setEnabled( false );
	deselectAll->setEnabled( false );

	KActionMenu *actActionMenu = actionCollection()->add<KActionMenu>( "rip" );
        actActionMenu->setText( i18n("&Rip") );
	actActionMenu->setDelayed(true); //needed for checking "all accounts"
	actActionMenu->setEnabled( false );
	connect(actActionMenu,SIGNAL(triggered()),tracks,SLOT(startSession()));

	ripMenu = actActionMenu->menu();
	connect(ripMenu, SIGNAL(triggered(QAction *)),this,SLOT(slotRipSelection(QAction *)));
	connect(ripMenu, SIGNAL(aboutToShow()),this,SLOT(setupRipMenu()));

	QAction *rip = actionCollection()->addAction("rip_selected");
	rip->setText(i18n("Rip &Selection"));
	connect(rip, SIGNAL(triggered(bool) ), tracks, SLOT(startSession()));
	rip->setEnabled( false );

	connect(this, SIGNAL(hasAudioCd(bool)), rip, SLOT(setEnabled(bool)));
	connect(this, SIGNAL(hasAudioCd(bool)), actActionMenu, SLOT(setEnabled(bool)));

	action = actionCollection()->addAction("clear_done_jobs");
        action->setText(i18n("Remove &Completed Jobs"));
	connect(action, SIGNAL(triggered(bool) ), jobQue, SLOT(clearDoneJobs()));

	QAction *edit = actionCollection()->addAction("edit_cd");
	edit->setText(i18n("&Edit Album..."));
	connect(edit, SIGNAL(triggered(bool) ), tracks, SLOT(editInformation()));
	connect(this, SIGNAL(hasAudioCd(bool)), edit, SLOT(setEnabled(bool)));
	edit->setEnabled( false );

// 	QAction *editTrack = actionCollection()->addAction("edit_track");
// 	editTrack->setText(i18n("Edit &Track"));
// 	connect(editTrack, SIGNAL(triggered(bool) ), tracks, SLOT(editCurrentTrack()));
// 	connect(tracks, SIGNAL(hasCD(bool)), editTrack, SLOT(setEnabled(bool)));
// 	editTrack->setEnabled( false );

	action = actionCollection()->addAction("encode_files");
        action->setText(i18n("Encode &Files from disk"));
	connect(action, SIGNAL(triggered(bool) ), SLOT(encodeFile()));

	QAction *cddb = actionCollection()->addAction("cddb_now");
	cddb->setText(i18n("&CDDB Lookup"));
	connect(cddb, SIGNAL(triggered(bool) ), tracks, SLOT(performCDDB()));
	connect(this, SIGNAL(hasAudioCd(bool)), cddb, SLOT(setEnabled(bool)));
	cddb->setEnabled( false );

	KStandardAction::configureNotifications(this, SLOT(configureNotifications()),
		  actionCollection());
	actionCollection()->addAction(KStandardAction::Quit, "quit", this, SLOT(close()));

	setupGUI();

	// Init statusbar
	// replace with something not hardcoded
	statusBar()->setContentsMargins(6, 0, 6, 0);
	driveLabel = new QLabel();
	statusBar()->addWidget(driveLabel);
	driveLabel->setText(i18n("Searching"));
	ripLabel = new QLabel();
	statusBar()->addPermanentWidget(ripLabel, 0);
	defaultEncLabel = new QLabel();
	statusBar()->addPermanentWidget(defaultEncLabel, 0);
	showCurrentEncoder();
	setRipStatus();
	// seems to need some time to settle
	QTimer::singleShot(50, tracks, SLOT(initDevice()));
}

void KAudioCreator::setDevice( const QString &device )
{
	tracks->setDevice(device);
}

void KAudioCreator::showJobPage()
{
	pageWidget->setCurrentPage(jobPage);
}

void KAudioCreator::slotRipSelection(QAction *selection) {
	tracks->startSession( (selection->data().toString() ));
}

void KAudioCreator::checkSettings()
{
	QStringList list = EncoderPrefs::prefsList();
	if (list.isEmpty()) {
		EncoderPrefs *encPrefs;
		encPrefs = EncoderPrefs::prefs("Encoder_WAV");
		encPrefs->setEncoderName(i18n("WAV"));
		encPrefs->setCommandLine("cp %f %o");
		encPrefs->setExtension("wav");
		encPrefs->setInputTypes("wav");
		encPrefs->setPercentLength(2);
		encPrefs->writeConfig();

		encPrefs = EncoderPrefs::prefs("Encoder_Ogg Default");
		encPrefs->setEncoderName(i18n("Ogg Default"));
		encPrefs->setCommandLine("oggenc -o %o --artist %{artist} --album %{albumtitle} --title %{title} --date %{year} --tracknum %{number} --genre %{genre} %f");
		encPrefs->setExtension("ogg");
		encPrefs->setInputTypes("wav");
		encPrefs->setPercentLength(4);
		encPrefs->writeConfig();

		encPrefs = EncoderPrefs::prefs("Encoder_MP3 Lame Standard");
		encPrefs->setEncoderName(i18n("MP3 Lame Standard"));
		encPrefs->setCommandLine("lame --preset standard --tt %{title} --ta %{artist} --tl %{albumtitle} --ty %{year} --tn %{number} --tg %{genre} %f %o");
		encPrefs->setExtension("mp3");
		encPrefs->setInputTypes("wav");
		encPrefs->setPercentLength(2);
		encPrefs->writeConfig();

		encPrefs = EncoderPrefs::prefs("Encoder_FLAC Best");
		encPrefs->setEncoderName(i18n("FLAC Best"));
		encPrefs->setCommandLine("flac --best -o %o --tag=Artist=%{artist} --tag=Album=%{albumtitle} --tag=Date=%{year} --tag=Title=%{title} --tag=Tracknumber=%{number} --tag=Genre=%{genre} %f");
		encPrefs->setExtension("flac");
		encPrefs->setInputTypes("wav");
		encPrefs->setPercentLength(2);
		encPrefs->writeConfig();

		Prefs::setInputTypesList(QStringList("wav"));
		Prefs::self()->writeConfig();
	} else if (!list.contains(QString("Encoder_WAV"))) {
		// what is this good for?
		// for the (admittedly rare) case WAV is manually erased from the config file
		EncoderPrefs *encPrefs;
		encPrefs = EncoderPrefs::prefs("Encoder_WAV");
		encPrefs->setEncoderName(i18n("WAV"));
		encPrefs->setCommandLine("cp %f %o");
		encPrefs->setExtension("wav");
		encPrefs->setInputTypes("wav");
		encPrefs->setPercentLength(2);
		encPrefs->writeConfig();

		QStringList types = Prefs::inputTypesList();
		if (!types.contains("wav")) {
			types.append("wav");
			Prefs::setInputTypesList(types);
		}
	}

	QString groupName = QString("Encoder_").append(Prefs::defaultEncoder());
	if (!list.contains(groupName)) {
		Prefs::setDefaultEncoder("WAV");
		Prefs::self()->writeConfig();
	}
}

void KAudioCreator::setupRipMenu()
{
	ripMenu->clear();

	const QStringList list = EncoderPrefs::prefsList();
	foreach (const QString &encoder, list) {
		EncoderPrefs *encPrefs = EncoderPrefs::prefs(encoder);
		if (encPrefs->inputTypes().contains("wav")) {
			const QString command = encPrefs->commandLine();
			int progEnd = command.indexOf(" ");
			const QString prog = command.left(progEnd).trimmed();
			if (KStandardDirs::findExe(prog) != QString()) {
				QAction *encAction = ripMenu->addAction(encPrefs->encoderName());
				encAction->setData(QVariant(encoder));
				if (encoder == QString("Encoder_").append(Prefs::defaultEncoder()))
					ripMenu->setDefaultAction(encAction);
			}
		}
    }
}

/**
 * Changes the status bar to let the user know if a cd was not detected or inserted.
 */
void KAudioCreator::setDriveStatus(AudioCD::DriveStatus status)
{
	if (driveLabel) {
		switch (status) {
			case AudioCD::NoDisc:
				driveLabel->setText(i18n("No disc"));
				emit hasAudioCd(FALSE);
				break;
			case AudioCD::Loading:
				driveLabel->setText(i18n("Loading disc"));
				break;
			case AudioCD::Ready:
				driveLabel->setText(i18n("Audio CD inserted"));
				emit hasAudioCd(TRUE);
				break;
			case AudioCD::ReadyNoAudio:
				driveLabel->setText(i18n("Disc inserted - No Audio"));
				break;
		}
	}
}

void KAudioCreator::showCurrentEncoder()
{
	QString encName = Prefs::defaultEncoder();
	defaultEncLabel->setText(i18n("Default encoder: %1", encName));
}

void KAudioCreator::setRipStatus()
{
	QString status = i18n("Idle");
	QString rippingStatus;
	QString encodingStatus;
	int activeRippingJobs = ripper->activeJobCount();
	int pendingRippingJobs = ripper->pendingJobCount();
	int activeEncodingJobs = encoder->activeJobCount();
	int pendingEncodingJobs = encoder->pendingJobCount();

	if ( activeRippingJobs ) {
		rippingStatus = i18n("Ripping (%1 active, %2 queued)", activeRippingJobs, pendingRippingJobs );
		status = rippingStatus;
	}
	if ( activeEncodingJobs ) {
		encodingStatus = i18n("Encoding (%1 active, %2 queued)", activeEncodingJobs, pendingEncodingJobs );

		if ( activeRippingJobs ) {
			status.append(" : ");
			status.append( encodingStatus );
		}
		else {
			status = encodingStatus;
		}
	}
	ripLabel->setText(i18n("Jobs:") + " " + status);
}

/**
 * Ask the user if they really want to quit if there are open jobs.
 */
bool KAudioCreator::queryClose() {
	if(jobQue->numberOfJobsNotFinished() > 0 &&
		(KMessageBox::warningContinueCancel(this, i18n("There are unfinished jobs in the queue. Would you like to quit anyway?"), i18n("Unfinished Jobs in Queue"),KStandardGuiItem::quit())
			  == KMessageBox::Cancel ))
		return false;
	return true;
}

void KAudioCreator::configureNotifications() {
    KNotifyConfigWidget::configure(this);
}

void KAudioCreator::encodeFile(){
	EncodeFileImp *file = new EncodeFileImp(this);
	connect(file, SIGNAL(startJob(Job*)),encoder, SLOT(encodeWav(Job*)));
	connect(file, SIGNAL(allJobsStarted()), this, SLOT(showJobPage()));
	file->exec();
}

/**
 * Show Settings dialog.
 */
void KAudioCreator::showSettings(){
	if(KConfigDialog::showDialog("settings"))
		return;

	SettingsDialog *dialog = new SettingsDialog(this, "settings", Prefs::self());
// 	connect(dialog, SIGNAL(settingsChanged(const QString &)), ripper, SLOT(loadSettings()));
	connect(dialog, SIGNAL(settingsChanged(const QString &)), encoder, SLOT(loadSettings()));
	connect(dialog, SIGNAL(settingsChanged(const QString &)), tracks, SLOT(loadSettings()));
	connect(dialog, SIGNAL(settingsChanged(const QString &)), this, SLOT(showCurrentEncoder()));
	connect(dialog, SIGNAL(settingsChanged(const QString &)), this, SLOT(setupRipMenu()));
	//custom signals
	connect(dialog->encoderConfigImp, SIGNAL(encoderChanged()), encoder, SLOT(loadSettings()));
	connect(dialog->encoderConfigImp, SIGNAL(encoderChanged()), this, SLOT(showCurrentEncoder()));
	connect(dialog->encoderConfigImp, SIGNAL(encoderChanged()), this, SLOT(setupRipMenu()));

	dialog->exec();
}

SettingsDialog::SettingsDialog(QWidget *parent, const char *name, KConfigSkeleton *config)
 : KConfigDialog(parent, name, config),
 cddb(0), cddbChanged(false)
{
	addPage(new General, i18n("General"), "kaudiocreator",
		  i18n("General Configuration"));

	addPage(new CdCfg, i18n("CD"), "media-optical-audio",
		  i18n("CD Configuration"));

	// Because WE don't segfault on our users...
	KService::Ptr libkcddb = KService::serviceByDesktopName("libkcddb");
	if (libkcddb && libkcddb->isValid())
	{
		cddb = KCModuleLoader::loadModule(QString("libkcddb"), KCModuleLoader::Inline);
		if (cddb)
		{
			cddb->load();
			addPage(cddb, i18n("CDDB"), "preferences-system-network", i18n("CDDB Configuration"), false);
			connect(cddb, SIGNAL(changed(bool)), this, SLOT(slotCddbChanged(bool)));
		}
	}
	RipCfg *rip = new RipCfg;
	rip->kcfg_tempDir->setMode(KFile::Directory);
	addPage(rip, i18n("Ripper"), "system-run", i18n("Ripper Configuration") );

	encoderConfigImp = new EncoderConfigImp;
	addPage(encoderConfigImp, i18n("Encoder"), "view-filter", i18n("Encoder Configuration") );
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
	//updateButtons();
}

#include "kaudiocreator.moc"

