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

#include <QKeyEvent>
#include <QPushButton>
#include <QTreeWidgetItemIterator>

#include <kmessagebox.h>
#include <kurl.h>
#include <kinputdialog.h>
#include <kprotocolinfo.h>
#include <kconfig.h>
#include <kapplication.h>
#include <kcombobox.h>
#include "libkcddb/genres.h"
#include "libkcddb/cdinfodialog.h"
#include "libkcompactdisc/kcompactdisc.h"
#include <kdebug.h>

#include "tracksimp.h"
#include "job.h"
#include "prefs.h"

using namespace KCDDB;

/**
 * Constructor, connect up slots and signals.
 */
TracksImp::TracksImp( QWidget *parent) : QWidget(parent), editedItem(0), cddbInfo()
{
	setupUi(this);
	trackListing->resizeColumnToContents(HEADER_RIP);
	cd = new KCompactDisc;

	genreBox->addItems(KCDDB::Genres().i18nList());

	connect(cd, SIGNAL(discChanged(unsigned int)), this, SLOT(newDisc(unsigned int)));

	connect(trackListing, SIGNAL(itemDoubleClicked(QTreeWidgetItem *, int)), this, SLOT(editTrackName(QTreeWidgetItem *)));
	connect(trackListing, SIGNAL(itemSelectionChanged()), this, SLOT(closeEditor()));
	connect(trackListing, SIGNAL(itemChanged(QTreeWidgetItem *, int)), this, SLOT(syncToCddbInfo(QTreeWidgetItem *, int)));
	connect(selectAllTracksButton, SIGNAL(clicked()), this, SLOT(selectAllTracks()));
	connect(deselectAllTracksButton, SIGNAL(clicked()), this, SLOT(deselectAllTracks()));

	connect(artistEdit, SIGNAL(editingFinished()), this, SLOT(artistChangedByUser()));
	connect(albumEdit, SIGNAL(editingFinished()), this, SLOT(albumChangedByUser()));
	connect(yearInput, SIGNAL(valueChanged(int)), this, SLOT(yearChangedByUser(int)));
	connect(genreBox, SIGNAL(currentIndexChanged(const QString &)), this, SLOT(genreChangedByUser(const QString &)));
	connect(genreBox, SIGNAL(editTextChanged(const QString &)), this, SLOT(genreChangedByUser(const QString &)));

	connect(deviceCombo, SIGNAL(currentIndexChanged(const QString &)), this, SLOT(changeDevice(const QString &)));

	toggleInputs(false);

	cddb = new KCDDB::Client();
	cddb->setBlockingMode(false);
	connect(cddb, SIGNAL(finished(KCDDB::Result)), this, SLOT(lookupCDDBDone(KCDDB::Result)));

	loadSettings();
}

/**
 * store the current device from the combo.
 */
TracksImp::~TracksImp()
{
	Prefs::setLastUsedDevice(deviceCombo->currentText());
	Prefs::self()->writeConfig();
}

/**
 * Load the class settings. 
 */
void TracksImp::loadSettings()
{
	deviceCombo->blockSignals(true);
	QStringList devices = KCompactDisc::cdromDeviceNames();
	deviceCombo->clear();

	if (devices.isEmpty()) {
		deviceCombo->addItem(i18n("none detected"));
	} else {
		foreach (const QString &tmpDevice, devices) {
			QString path = KCompactDisc::cdromDeviceUrl(tmpDevice).path();
			deviceCombo->addItem(tmpDevice + " (" + path + ")");
		}
	}

	int i = deviceCombo->findText(Prefs::lastUsedDevice());
	if (i >= 0) deviceCombo->setCurrentIndex(i);
	deviceCombo->blockSignals(false);
}

void TracksImp::initDevice()
{
	changeDevice(deviceCombo->currentText());
}

void TracksImp::newDisc(unsigned tracks)
{
	if (!tracks)
	{
		kDebug(60002) << "newDisc - No disc";
		cddbInfo.clear();
		newAlbum();
		emit(hasCD(false));

		toggleInputs(false);

		return;
	}
	
	unsigned discId = cd->discId();
	kDebug(60002) << "newDisc - " << discId;
	emit(hasCD(true));

	toggleInputs(true);
 
	cddbInfo.clear();

	cddbInfo.set("discid", QString::number(discId,16).rightJustified(8,'0'));
	cddbInfo.set(Length, cd->discLength());

	cddbInfo.set(Artist, cd->discArtist());
	cddbInfo.set(Title, cd->discTitle());

	// If it's a sampler, we'll do artist/title.
	bool isSampler = (cddbInfo.get(Title).toString().compare("Various") == 0);
	for (unsigned i = 1; i <= cd->tracks(); i++) {
		TrackInfo& track(cddbInfo.track(i-1));
		if (isSampler)
			track.set(Artist, cd->trackArtist(i));
		track.set(Title, cd->trackTitle(i));
	}

	newAlbum();

	if (Prefs::performCDDBauto())
		lookupCDDB();
}

/**
 * @return if there is a cd inserted or not.
 */
bool TracksImp::hasCD()
{
	return !cd->isNoDisc();
}

/**
 * If the user gave a drive on the commandline
 * assume KCompactDisc::cdromDeviceNames() listed all valid drives
 * and try to match one
 */
void TracksImp::setDevice(const QString &userDevice)
{
	KUrl url = KCompactDisc::cdromDeviceUrl(userDevice);
	bool found = false;
	int d = 0;
	while (d < deviceCombo->count()) {
		QString name = deviceCombo->itemText(d);
		name = name.left(name.indexOf(" ("));
		if (KCompactDisc::cdromDeviceUrl(name) == url) {
			if (d != deviceCombo->currentIndex()) {
				changeDevice(name);
				deviceCombo->blockSignals(true);
				deviceCombo->setCurrentIndex(d);
				deviceCombo->blockSignals(false);
			}
			found = true;
			break;
		}
		++d;
	}
	if (!found) kDebug() << "Selected device not found!" << endl;
}

/**
 * The device text has changed.
 * @param device - the new text to check.
 */
void TracksImp::changeDevice(const QString &device)
{
	QString newDevice = device.left(device.indexOf(" ("));
	// assume that devices reported by KCompactDisc::cdromDeviceNames() are just there
	if (!cd->setDevice(newDevice, 50, false))
	{
		QString errstring =
		  i18n("CDROM read or access error (or no audio disk in drive).\n"\
		    "Please make sure you have access permissions to:\n%1",
		     device);
		KMessageBox::error(this, errstring, i18n("Error"));
	}
}

/**
 * Helper function (toolbar button) for users.
 **/ 
void TracksImp::performCDDB() {
	if (!hasCD()) {
		KMessageBox::sorry(this, i18n("Please insert a disk."),
		       i18n("CDDB Failed"));
		return;
	}

	lookupCDDB();
}

/**
 * See if we can't get the cddb value for this cd.
 */ 
void TracksImp::lookupCDDB() {
	cddb->config().reparse();
	cddb->lookup(cd->discSignature());
}

/**
 * The non blocking CDDB function calling has finished.	Report an error or
 * continue.
 * @param result the success or failure of the cddb retrieval.
 */
void TracksImp::lookupCDDBDone(Result result ) {
	if ((result != KCDDB::Success) &&
		(result != KCDDB::MultipleRecordFound))
	{
		KMessageBox::sorry(this, i18n("Unable to retrieve CDDB information."), i18n("CDDB Failed"));
		return;
	}

	// Choose the cddb entry
	KCDDB::CDInfo info = cddb->lookupResponse().first();
	// TODO Why doesn't libcddb not return MultipleRecordFound?
	//if( result == KCDDB::MultipleRecordFound ) {
	if( Prefs::promptIfIncompleteInfo() && cddb->lookupResponse().count() > 1 ) {
		QString searchedCDId = cddbInfo.get("discid").toString();
		CDInfoList cddb_info = cddb->lookupResponse();
		CDInfoList::iterator it;
		QStringList list;
		for ( it = cddb_info.begin(); it != cddb_info.end(); ++it  ) {
			list.append( QString("%1, %2, %3").arg((*it).get(Artist).toString()).arg((*it).get(Title).toString())
			  .arg((*it).get(Genre).toString()));
		}

		bool ok(false); 
		QString res = KInputDialog::getItem(
						i18n("Select CDDB entry"),
						i18n("Select a CDDB entry:"), list, 0, false, &ok,
						this );
		if ( ok ) {
			// The user selected and item and pressed OK
			int c = 0;
			for ( QStringList::Iterator it = list.begin(); it != list.end(); ++it ) {
				if( *it == res)	break;
				c++;
			}
			if( c < cddb_info.size() )
				info = cddb_info[c];
		} else {
			return;
			// user pressed Cancel
		}
	// Check that the CD we looked up is the one now loaded.
	// The user might have ejected the CD while we were in the
	// KInputDialog event loop, and replaced it with another one.
		if ( searchedCDId != cddbInfo.get("discid").toString() )
			return;
	}

	Q_ASSERT(info.numberOfTracks() == cddbInfo.numberOfTracks());
	cddbInfo = info;
	newAlbum();

	// See if the user wishes to automatically rip after successfully retrieving
	if( Prefs::autoRip())
		ripWholeAlbum();
}

void TracksImp::artistChangedByUser()
{
	cddbInfo.set(Artist, artistEdit->text());
	setAlbumInfo(cddbInfo.get(Artist).toString(), cddbInfo.get(Title).toString());
}

void TracksImp::albumChangedByUser()
{
	cddbInfo.set(Title, albumEdit->text());
	setAlbumInfo(cddbInfo.get(Artist).toString(), cddbInfo.get(Title).toString());
}

void TracksImp::yearChangedByUser(int newYear)
{
	cddbInfo.set(Year, newYear);
}

void TracksImp::genreChangedByUser(const QString &newGenre)
{
	cddbInfo.set(Genre, newGenre);
}

/**
 * Bring up the dialog to edit the information about this album.
 * If there is not currently selected track return.
 * If ok is pressed then store the information and update track name.
 */
void TracksImp::editInformation() {
	if( !hasCD() ) return;
	// Create dialog.
	CDInfoDialog *dialog = new CDInfoDialog( this );
	dialog->setModal(false);
	dialog->setCaption(i18n( "CD Editor" ));
	dialog->setButtons(KDialog::Ok|KDialog::Cancel);
	dialog->setDefaultButton(KDialog::Ok);
	dialog->showButtonSeparator(true);

	dialog->setInfo(cddbInfo, cd->discSignature());

	// Show dialog->and save results.
	bool okClicked = dialog->exec();
	if( okClicked ) {
		cddbInfo = dialog->info();
		newAlbum();
		cddb->store(cddbInfo,cd->discSignature());
	}
	delete dialog;
}

QString TracksImp::formatTime(unsigned s)
{
	QTime time;

	time = time.addSecs((int) s);

	// Use ".zzz" for milliseconds...
	QString temp2;
	if (time.hour() > 0)
		temp2 = time.toString("hh:mm:ss");
	else
		temp2 = time.toString("mm:ss");
	return temp2;
}

/**
 * Helper function.
 * Selects all tracks and then calls startSession to rip them all.
 */
void TracksImp::ripWholeAlbum() {
	selectAllTracks();
	startSession();
}

/**
 * Start of the "ripping session" by emiting signals to rip the selected tracks.
 * If any album information is not set, notify the user first.
 */
void TracksImp::startSession( QString encoder )
{
	QString currentEncoder = encoder;
	if (currentEncoder.isEmpty())
		currentEncoder = Prefs::defaultEncoder();

	QList<TracksItem *> selected = selectedTracks();

	if( selected.isEmpty() )
	{
		int i = KMessageBox::questionYesNo( this, i18n("No tracks have been selected.  Would you like to rip the entire CD?"),
					                        i18n("No Tracks Selected"), KGuiItem(i18n("Rip CD")), KStandardGuiItem::cancel() );
		if( i == KMessageBox::No )
			return;

		selectAllTracks();
		selected = selectedTracks();
	}

	if (!KProtocolInfo::isKnownProtocol("audiocd"))
	{
		KMessageBox::sorry( this, i18n("Could not find audiocd:/ protocol. Please install the audiocd ioslave"),
		                          i18n("Protocol Not Found"));
		return;
	}

	QStringList list;
	if( cddbInfo.get(Genre).toString() == QLatin1String("Unknown") )
		list += "Genre";
	if( cddbInfo.get(Year).toInt() == 0 )
		list += "Year";
	if( cddbInfo.get(Artist).toString() == QLatin1String("Unknown Artist"))
		list += "Artist";
	if( cddbInfo.get(Title).toString() == QLatin1String("Unknown Album"))
		list += "Album";

	if( Prefs::promptIfIncompleteInfo() && list.count()>0 ) {
		int r = KMessageBox::questionYesNo(this, i18n("Part of the album is not set: %1.\n (To change album information click the \"Edit Information\" button.)\n Would you like to rip the selected tracks anyway?", list.join(", ")), i18n("Album Information Incomplete"),KGuiItem(i18n("Rip")),KStandardGuiItem::cancel());
		if( r == KMessageBox::No )
			return;
	}
	
	Job *lastJob = 0;
	foreach (TracksItem* item, selected) {
		Job *newJob = new Job();
		newJob->encoder = currentEncoder;
		newJob->device = cd->deviceUrl().path();
		newJob->album = cddbInfo.get(Title).toString();
		newJob->genre = cddbInfo.get(Genre).toString();
		if( newJob->genre.isEmpty())
			newJob->genre = "Pop";
		newJob->group = cddbInfo.get(Artist).toString();
		newJob->comment = cddbInfo.get(Comment).toString();
		newJob->year = cddbInfo.get(Year).toInt();
		newJob->track = item->track();

		//newJob->track_title = item->title();
		newJob->track_title = item->text(HEADER_TRACK_NAME);
		newJob->track_artist = item->artist();
		newJob->track_comment = item->comment();
		lastJob = newJob;
		emit( ripTrack(newJob) ); 
	}

	if (lastJob)
		lastJob->lastSongInAlbum = true;

	KMessageBox::information(this,
	i18n("%1 Job(s) have been started.  You can watch their progress in the "\
	   "jobs section.", selected.count()),
	i18n("Jobs have started"), i18n("Jobs have started"));
}

QList<TracksItem *> TracksImp::selectedTracks()
{
	QList<TracksItem *> selected;

	QTreeWidgetItemIterator it(trackListing, QTreeWidgetItemIterator::Checked);
	while (*it) {
		TracksItem *item = (TracksItem *)(*it);
		selected.append( item );
		++it;
	}
	return selected;
}

/**
 * Turn on all of the tracks.
 */
void TracksImp::selectAllTracks() {
	QTreeWidgetItemIterator it(trackListing);
	while (*it) {
		(*it)->setCheckState( HEADER_RIP, Qt::Checked );
		++it;
	}
}

/**
 * Turn off all of the tracks.
 */
void TracksImp::deselectAllTracks() {
	QTreeWidgetItemIterator it(trackListing);
	while (*it) {
		(*it)->setCheckState( HEADER_RIP, Qt::Unchecked );
		++it;
	}
}

void TracksImp::toggleInputs(bool status)
{
	selectAllTracksButton->setEnabled(status);
	deselectAllTracksButton->setEnabled(status);

	artistEdit->setEnabled(status);
	albumEdit->setEnabled(status);
	yearInput->setEnabled(status);
	genreBox->setEnabled(status);
}

/**
 * Set the current stats for the new album being displayed.
 */
void TracksImp::newAlbum() {
	QString albumTitle = cddbInfo.get(Title).toString();
	QString albumArtist = cddbInfo.get(Artist).toString();

	artistEdit->setText(albumArtist);
	albumEdit->setText(albumTitle);
	setAlbumInfo(albumArtist, albumTitle);
	yearInput->setValue(cddbInfo.get(Year).toInt());
	genreBox->setEditText(cddbInfo.get(Genre).toString());
	trackListing->clear();
	toggleInputs(false);
	emit(hasTracks(false));

	for (int i = 0; i < cddbInfo.numberOfTracks(); i++)
	{
		TrackInfo ti = cddbInfo.track(i);
		// There is a new track for this title.  Add it to the list of tracks.
		QString trackLength = formatTime(cd->trackLength(i+1));
		TracksItem *newItem = new TracksItem(0, ti.get(Title).toString(), ti.get(Artist).toString(),
						      i+1, trackLength, ti.get(Comment).toString());
		newItem->setCheckState(HEADER_RIP, Qt::Unchecked);
		newItem->setTextAlignment(1, Qt::AlignHCenter);
		newItem->setText(HEADER_TRACK, QString::number(i+1));
		newItem->setTextAlignment(HEADER_LENGTH, Qt::AlignHCenter);
		newItem->setText(HEADER_LENGTH, trackLength);
		newItem->setText(HEADER_TRACK_NAME, ti.get(Title).toString());
		trackListing->addTopLevelItem(newItem);
	}

	trackListing->resizeColumnToContents(HEADER_TRACK);
	trackListing->resizeColumnToContents(HEADER_LENGTH);

	if (cddbInfo.numberOfTracks())
	{
		// Set the current selected track to the first one.
		trackListing->setCurrentItem(trackListing->topLevelItem(0));
		toggleInputs(true);
		emit(hasTracks(true));
	}
}

void TracksImp::setAlbumInfo(const QString &artist, const QString &album)
{
	QString albumInfo;
	if (cddbInfo.isValid())
		albumInfo = artist.isEmpty() ? album : artist + i18n( " - " ) + album;
	else
		albumInfo = i18n("No disc");

	albumInfoLabel->setText(albumInfo);
}

/**
 * If the user presses the F2 key, trigger renaming of the title.
 * @param event the QKeyEvent passed to this event handler.
 */
// void TracksImp::keyPressEvent(QKeyEvent *event) {
// kDebug() << "Enter" << endl;
// 	QTreeWidgetItem *item = trackListing->currentItem();
// 	if( !item ) return;
//  	if( event->key() == Qt::Key_Enter )
//  	{
// kDebug() << "Enter" << endl;
//  //		emit(renameTrack(item));
//  ///TODO		item->setRenameEnabled( HEADER_TRACK_NAME, true );
//  ///TODO		item->startRename( HEADER_TRACK_NAME );
//  		event->accept();
//  	}
//  	else
//  		keyPressEvent(event);
//  }

void TracksImp::editTrackName(QTreeWidgetItem *item)
{
	if (!editedItem && item) {
		trackListing->openPersistentEditor(item, HEADER_TRACK_NAME);
		editedItem = item;
	} else if (editedItem && item && (editedItem != item)) {
		trackListing->closePersistentEditor(editedItem, HEADER_TRACK_NAME);
		trackListing->openPersistentEditor(item, HEADER_TRACK_NAME);
		editedItem = item;
	} else if (editedItem && item && (editedItem == item)) {
		trackListing->closePersistentEditor(editedItem, HEADER_TRACK_NAME);
		editedItem = 0;
	}
}

void TracksImp::closeEditor()
{
	if (editedItem) {
		trackListing->closePersistentEditor(editedItem, HEADER_TRACK_NAME);
		editedItem = 0;
	}
}

void TracksImp::syncToCddbInfo(QTreeWidgetItem *item, int /*column*/)
{
	TrackInfo &track = cddbInfo.track((item->text(HEADER_TRACK)).toInt() - 1);
	track.set(Title, QVariant(item->text(HEADER_TRACK_NAME)));
}

/**
 * Eject the current cd device
 */
void TracksImp::eject() {
	cd->eject();
}

/**
 * Eject a device
 * @param deviceToEject the device to eject.
 */
void TracksImp::ejectDevice(const QString &deviceToEject) {
	changeDevice(deviceToEject);
	
	cd->eject();
}

#include "tracksimp.moc"

