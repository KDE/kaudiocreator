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

#include <QPushButton>
#include <QStandardItemModel>

#include <kmessagebox.h>
#include <kurl.h>
#include <kinputdialog.h>
#include <kprotocolinfo.h>
#include <kconfig.h>
#include <kapplication.h>
#include <kcombobox.h>
#include <KCddb/Genres>
#include <KCddb/CdinfoDialog>
#include <kdebug.h>
#include <solid/device.h>
#include <solid/opticaldrive.h>
#include <solid/devicenotifier.h>

#include "job.h"
#include "tracksimp.h"
#include "defs.h"
#include "prefs.h"

using namespace KCDDB;

/**
 * Constructor, connect up slots and signals.
 */
TracksImp::TracksImp( QWidget *parent) : QWidget(parent), currentDrive(0), cddbInfo()
{
    setupUi(this);
    trackModel = new QStandardItemModel(0, 6, this);
    trackModel->setHorizontalHeaderLabels(QStringList() << i18nc("@title:column", "Rip") << i18n("Track") << i18n("Length") << i18n("Title") << i18n("Artist") << i18n("Comment"));
    trackView->setModel(trackModel);
    trackView->resizeColumnToContents(COLUMN_RIP);

	genreBox->addItems(KCDDB::Genres().i18nList());

	connect(trackModel, SIGNAL(itemChanged(QStandardItem *)), this, SLOT(syncToCddbInfo(QStandardItem *)));
	connect(selectAllTracksButton, SIGNAL(clicked()), this, SLOT(selectAllTracks()));
	connect(deselectAllTracksButton, SIGNAL(clicked()), this, SLOT(deselectAllTracks()));

    connect(artistEdit, SIGNAL(editingFinished()), this, SLOT(artistChangedByUser()));
    connect(albumEdit, SIGNAL(editingFinished()), this, SLOT(albumChangedByUser()));
    connect(commentEdit, SIGNAL(editingFinished()), this, SLOT(commentChangedByUser()));

    connect(artistEditButton, SIGNAL(clicked()), this, SLOT(assignArtisToTracks()));
    connect(commentEditButton, SIGNAL(clicked()), this, SLOT(assignCommentToTracks()));
    
	connect(yearInput, SIGNAL(valueChanged(int)), this, SLOT(yearChangedByUser(int)));
	connect(genreBox, SIGNAL(currentIndexChanged(const QString &)), this, SLOT(genreChangedByUser(const QString &)));
	connect(genreBox, SIGNAL(editTextChanged(const QString &)), this, SLOT(genreChangedByUser(const QString &)));

	connect(deviceCombo, SIGNAL(currentIndexChanged(const QString &)), this, SLOT(changeDevice(const QString &)));

	toggleInputs(false);

	cddb = new KCDDB::Client();
	cddb->setBlockingMode(false);
	connect(cddb, SIGNAL(finished(KCDDB::Result)), this, SLOT(lookupCDDBDone(KCDDB::Result)));

	loadSettings();

    bell = Solid::DeviceNotifier::instance();
    connect(bell, SIGNAL(deviceAdded(const QString &)), this, SLOT(registerDevice(const QString &)));
    connect(bell, SIGNAL(deviceRemoved(const QString &)), this, SLOT(unregisterDevice(const QString &)));
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

    devMap.clear();
    // a lite version of KCompactDisc::cdromDeviceNames()
    foreach (const Solid::Device &device, Solid::Device::listFromType(Solid::DeviceInterface::OpticalDrive)) {
        kDebug() << device.udi().toLatin1().constData();
        const Solid::Block *b = device.as<Solid::Block>();
        QString devKey = b->device() + " (" + device.vendor() + " " + device.product() + ")";
        devMap.insert(devKey, device);
        udiMap.insert(device.udi(), devKey);
    }

    deviceCombo->clear();

	if (udiMap.isEmpty()) {
		deviceCombo->addItem(i18n("none detected"));
	} else {
		deviceCombo->addItems(devMap.keys());
	}

	int i = deviceCombo->findText(Prefs::lastUsedDevice());
	if (i >= 0) deviceCombo->setCurrentIndex(i);
	deviceCombo->blockSignals(false);
}

void TracksImp::initDevice()
{
	changeDevice(deviceCombo->currentText());
}

void TracksImp::newDisc()
{
	toggleInputs(true);
 
	cddbInfo.clear();

	cddbInfo.set("discid", currentDrive->getFreeDbId());
	cddbInfo.set(Length, currentDrive->getDiscLength());
 
	// If it's a sampler, we'll do artist/title.
	bool isSampler = (cddbInfo.get(Title).toString().compare("Various") == 0);
	for (unsigned i = 1; i <= currentDrive->getTrackNum(); i++) {
		TrackInfo& track(cddbInfo.track(i-1));
		if (isSampler)
			track.set(Artist, QString());
		track.set(Title, QString());
	}

    if (Prefs::performCDDBauto()) {
        lookupCDDB();
    } else {
        newAlbum();
    }
}

void TracksImp::discChanged(AudioCD::DriveStatus status)
{
    switch (status) {
        case AudioCD::NoDrive:
            artistEdit->setText(QString());
            albumEdit->setText(QString());
            commentEdit->setText(QString());
            setAlbumInfo(i18n("Unknown Artist"), i18n("Unknown Album"));
            yearInput->setValue(0);
            genreBox->setEditText(QString());
            trackModel->clear();
            trackModel->setHorizontalHeaderLabels(QStringList() << i18nc("@title:column", "Rip") << i18n("Track") << i18n("Length") << i18n("Title") << i18n("Artist") << i18n("Comment"));
            toggleInputs(false);
            emit driveStatusChanged(AudioCD::NoDrive);
            break;
        case AudioCD::NoDisc:
            artistEdit->setText(QString());
            albumEdit->setText(QString());
            commentEdit->setText(QString());
            setAlbumInfo(i18n("Unknown Artist"), i18n("Unknown Album"));
            yearInput->setValue(0);
            genreBox->setEditText(QString());
            trackModel->clear();
            trackModel->setHorizontalHeaderLabels(QStringList() << i18nc("@title:column", "Rip") << i18n("Track") << i18n("Length") << i18n("Title") << i18n("Artist") << i18n("Comment"));
            toggleInputs(false);
            emit driveStatusChanged(AudioCD::NoDisc);
            break;
        case AudioCD::Loading:
            emit driveStatusChanged(AudioCD::Loading);
            break;
        case AudioCD::Ready:
            emit driveStatusChanged(AudioCD::Ready);
            newDisc();
            break;
        case AudioCD::ReadyNoAudio:
            emit driveStatusChanged(AudioCD::ReadyNoAudio);
            break;
    }
}

/**
 * @return if there is a cd inserted or not.
 */
bool TracksImp::hasCD()
{
    if (currentDrive)
        return currentDrive->isCdInserted();
    else
        return false;
}

bool TracksImp::hasAudio() const
{
    if (currentDrive)
        return currentDrive->hasAudio();
    else
        return false;
}

/**
 * If the user gave a drive on the commandline
 * assume KCompactDisc::cdromDeviceNames() listed all valid drives
 * and try to match one
 */
void TracksImp::setDevice(const QString &userDevice)
{
    bool found = false;
    int d = 0;
    while (d < deviceCombo->count()) {
        QString name = deviceCombo->itemText(d);
        if (name.contains(userDevice)) {
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
    if (!found) kDebug() << "Selected device not found!";
}

/**
 * The device text has changed.
 * @param device - the new text to check.
 */
void TracksImp::changeDevice(const QString &device)
{
    delete currentDrive;
    if (!udiMap.isEmpty()) {
        currentDrive = new AudioCD();
        if (currentDrive->setDevice(devMap[device])) {
            connect(currentDrive, SIGNAL(driveStatusChanged(AudioCD::DriveStatus)), this, SLOT(discChanged(AudioCD::DriveStatus)));
        }
        discChanged(currentDrive->getDriveStatus());
    } else {
        currentDrive = 0;
        discChanged(AudioCD::NoDrive);
    }
}

void TracksImp::registerDevice(const QString &udi)
{
    Solid::Device nd(udi);
    if (nd.isDeviceInterface(Solid::DeviceInterface::OpticalDrive) && !udiMap.contains(udi)) {
        const Solid::Block *b = nd.as<Solid::Block>();
        QString devKey = b->device() + " (" + nd.vendor() + " " + nd.product() + ")";
        devMap.insert(devKey, nd);
        udiMap.insert(udi, devKey);
        deviceCombo->addItem(devKey);
        int i = deviceCombo->findText(i18n("none detected"));
        if (i != -1)
            deviceCombo->removeItem(i);
    }
}

void TracksImp::unregisterDevice(const QString &udi)
{
    if (udiMap.contains(udi)) {
        deviceCombo->removeItem(deviceCombo->findText(udiMap[udi]));
        devMap.remove(udiMap[udi]);
        udiMap.remove(udi);
    }

    if (udiMap.isEmpty()) {
        deviceCombo->addItem(i18n("none detected"));
        devMap.clear(); // sometimes devMap still contains a "" entry?
    }
}

/**
 * Helper function (toolbar button) for users.
 **/ 
void TracksImp::performCDDB()
{
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
void TracksImp::lookupCDDB()
{
    cddb->config().reparse();
    if (currentDrive) cddb->lookup(currentDrive->getOffsetList());
}

/**
 * The non blocking CDDB function calling has finished.	Report an error or
 * continue.
 * @param result the success or failure of the cddb retrieval.
 */
void TracksImp::lookupCDDBDone(Result result )
{
	if ((result != KCDDB::Success) &&
		(result != KCDDB::MultipleRecordFound))
	{
		KMessageBox::sorry(this, i18n("Unable to retrieve CDDB information."), i18n("CDDB Failed"));
        newAlbum();
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
    cddb->store(cddbInfo, currentDrive->getOffsetList());
    setAlbumInfo(cddbInfo.get(Artist).toString(), cddbInfo.get(Title).toString());
}

void TracksImp::albumChangedByUser()
{
    cddbInfo.set(Title, albumEdit->text());
    cddb->store(cddbInfo,currentDrive->getOffsetList());
    setAlbumInfo(cddbInfo.get(Artist).toString(), cddbInfo.get(Title).toString());
}

void TracksImp::commentChangedByUser()
{
    cddbInfo.set(Comment, commentEdit->text());
    cddb->store(cddbInfo,currentDrive->getOffsetList());
}

void TracksImp::assignArtisToTracks()
{
    int rows = trackModel->rowCount();
    for (int r = 0; r < rows; ++r) {
        trackModel->item(r, COLUMN_TRACK_ARTIST)->setData(artistEdit->text(), Qt::DisplayRole);
    }
}

void TracksImp::assignCommentToTracks()
{
    int rows = trackModel->rowCount();
    for (int r = 0; r < rows; ++r) {
        trackModel->item(r, COLUMN_TRACK_COMMENT)->setData(commentEdit->text(), Qt::DisplayRole);
    }
}

void TracksImp::yearChangedByUser(int newYear)
{
    cddbInfo.set(Year, newYear);
    if (currentDrive) cddb->store(cddbInfo, currentDrive->getOffsetList());
}

void TracksImp::genreChangedByUser(const QString &newGenre)
{
    cddbInfo.set(Genre, newGenre);
    if (currentDrive) cddb->store(cddbInfo, currentDrive->getOffsetList());
}

/**
 * Bring up the dialog to edit the information about this album.
 * If there is not currently selected track return.
 * If ok is pressed then store the information and update track name.
 */
void TracksImp::editInformation()
{
    if( !hasCD() ) return;
	// Create dialog.
	CDInfoDialog *dialog = new CDInfoDialog( this );
	dialog->setModal(false);
	dialog->setWindowTitle(i18n( "CD Editor" ));
	dialog->setInfo(cddbInfo, currentDrive->getOffsetList());

	// Show dialog->and save results.
	bool okClicked = dialog->exec();
	if( okClicked ) {
        cddbInfo = dialog->info();
		newAlbum();
		cddb->store(cddbInfo, currentDrive->getOffsetList());
	}
	delete dialog;
}

QString TracksImp::formatTime(unsigned s)
{
	QTime time;
	time = time.addMSecs((int) s);

	if (time.hour() > 0)
		return time.toString("hh:mm:ss");
	else
		return time.toString("mm:ss");
}

/**
 * Helper function.
 * Selects all tracks and then calls startSession to rip them all.
 */
void TracksImp::ripWholeAlbum()
{
	selectAllTracks();
	startSession();
}

/**
 * Start of the "ripping session" by emiting signals to rip the selected tracks.
 * If any album information is not set, notify the user first.
 */
void TracksImp::startSession(QString encoder)
{
	QString currentEncoder = encoder;
	if (currentEncoder.isEmpty())
		currentEncoder = QString("Encoder_").append(Prefs::defaultEncoder());

	QList<int> selected = selectedTracks();

	if( selected.isEmpty() ) {
		int i = KMessageBox::questionYesNo( this, i18n("No tracks have been selected.  Would you like to rip the entire CD?"),
					                        i18n("No Tracks Selected"), KGuiItem(i18n("Rip CD")), KStandardGuiItem::cancel() );
		if( i == KMessageBox::No )
			return;

		selectAllTracks();
		selected = selectedTracks();
	}

	if (!KProtocolInfo::isKnownProtocol("audiocd")) {
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
	foreach (int r, selected) {
		Job *newJob = new Job();
		newJob->encoder = currentEncoder;
		newJob->device = currentDrive->getCdPath();
		newJob->driveUdi = currentDrive->getDriveUdi();
		newJob->album = cddbInfo.get(Title).toString();
		newJob->genre = cddbInfo.get(Genre).toString();
		if( newJob->genre.isEmpty())
			newJob->genre = "Pop";
		newJob->group = cddbInfo.get(Artist).toString();
		newJob->comment = cddbInfo.get(Comment).toString();
		newJob->year = cddbInfo.get(Year).toInt();
		newJob->track = (trackModel->item(r, COLUMN_TRACK))->data(Qt::DisplayRole).toInt();
		newJob->track_title = (trackModel->item(r, COLUMN_TRACK_NAME))->data(Qt::DisplayRole).toString();
		newJob->track_artist = (trackModel->item(r, COLUMN_TRACK_ARTIST))->data(Qt::DisplayRole).toString();
		newJob->track_comment = (trackModel->item(r, COLUMN_TRACK_COMMENT))->data(Qt::DisplayRole).toString();
		lastJob = newJob;
		emit( ripTrack(newJob) ); 
	}

	if (lastJob)
		lastJob->lastSongInAlbum = true;

	KMessageBox::information(this,
	i18n("%1 Job(s) have been started.  You can watch their progress in the "\
	   "jobs section.", selected.count()),
	i18n("Jobs have started"), i18n("Jobs have started"));

	emit(sessionStarted());
}

QList<int> TracksImp::selectedTracks()
{
	QList<int> selected;

    int rows = trackModel->rowCount();
    for (int r = 0; r < rows; ++r) {
		if ((trackModel->item(r, COLUMN_RIP)->checkState()) == Qt::Checked)
            selected.append(r);
	}

	return selected;
}

/**
 * Turn on all of the tracks.
 */
void TracksImp::selectAllTracks()
{
    int rows = trackModel->rowCount();
    for (int r = 0; r < rows; ++r) {
		trackModel->item(r, COLUMN_RIP)->setCheckState(Qt::Checked);
	}
}

/**
 * Turn off all of the tracks.
 */
void TracksImp::deselectAllTracks()
{
    int rows = trackModel->rowCount();
    for (int r = 0; r < rows; ++r) {
        trackModel->item(r, COLUMN_RIP)->setCheckState(Qt::Unchecked);
    }
}

void TracksImp::toggleInputs(bool status)
{
	selectAllTracksButton->setEnabled(status);
	deselectAllTracksButton->setEnabled(status);

	artistEdit->setEnabled(status);
    artistEditButton->setEnabled(status);
	albumEdit->setEnabled(status);
    commentEdit->setEnabled(status);
    commentEditButton->setEnabled(status);
	yearInput->setEnabled(status);
	genreBox->setEnabled(status);
}

/**
 * Set the current stats for the new album being displayed.
 */
void TracksImp::newAlbum()
{
	QString albumTitle = cddbInfo.get(Title).toString();
	QString albumArtist = cddbInfo.get(Artist).toString();

	artistEdit->setText(albumArtist);
	albumEdit->setText(albumTitle);
    commentEdit->setText(cddbInfo.get(Comment).toString());
	setAlbumInfo(albumArtist, albumTitle);
	yearInput->setValue(cddbInfo.get(Year).toInt());
	genreBox->setEditText(cddbInfo.get(Genre).toString());
	trackModel->clear();
    trackModel->setHorizontalHeaderLabels(QStringList() << i18nc("@title:column", "Rip") << i18n("Track") << i18n("Length") << i18n("Title") << i18n("Artist") << i18n("Comment"));
	toggleInputs(false);
	emit(hasTracks(false));

	for (uint i = 0; i < currentDrive->getTrackNum(); ++i) {
	TrackInfo ti = cddbInfo.track(i);
		// There is a new track for this title.  Add it to the list of tracks.
		QString trackLength = formatTime(currentDrive->getTrackLength(i));
        QList<QStandardItem *> trackItems = QList<QStandardItem *>();
        
		QStandardItem *ripItem = new QStandardItem();
        ripItem->setCheckable(true);
        trackItems << ripItem;

        QStandardItem *trackItem = new QStandardItem(QString::number(i + 1));
        trackItem->setEditable(false);
        trackItems << trackItem;
        
        QStandardItem *lengthItem = new QStandardItem(trackLength);
        lengthItem->setEditable(false);
        trackItems << lengthItem;
        
        QStandardItem *titleItem = new QStandardItem(ti.get(Title).toString());
        trackItems << titleItem;

        QStandardItem *artistItem = new QStandardItem(ti.get(Artist).toString());
        trackItems << artistItem;

        QString trackComment = ti.get(Comment).toString();
        if (trackComment.isEmpty() && !(cddbInfo.get(Comment).toString().isEmpty()))
            trackComment = cddbInfo.get(Comment).toString();
        QStandardItem *commentItem = new QStandardItem(trackComment);
        trackItems << commentItem;

        trackModel->appendRow(trackItems);
	}

    trackView->resizeColumnToContents(COLUMN_RIP);
	trackView->resizeColumnToContents(COLUMN_TRACK);
	trackView->resizeColumnToContents(COLUMN_LENGTH);
    trackView->resizeColumnToContents(COLUMN_TRACK_NAME);
    trackView->resizeColumnToContents(COLUMN_TRACK_ARTIST);

	if (currentDrive->getTrackNum()) {
		// Set the current selected track to the first one.
		trackView->setCurrentIndex(trackModel->index(0, 0, QModelIndex()));
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

void TracksImp::syncToCddbInfo(QStandardItem *item)
{
    if (item->column() == COLUMN_TRACK_NAME) {
        TrackInfo &track = cddbInfo.track(item->row());
        track.set(Title, item->data(Qt::DisplayRole));
    }
}

/**
 * Eject the current cd device
 */
void TracksImp::eject()
{
    currentDrive->eject();
}

/**
 * Eject a device
 * @param deviceToEject the device to eject.
 */
void TracksImp::ejectDevice(const QString &deviceToEject)
{
    Solid::Device device(deviceToEject);
    Solid::OpticalDrive *drive = device.as<Solid::OpticalDrive>();
    drive->eject();
}

#include "tracksimp.moc"

