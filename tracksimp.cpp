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
#include "tracksimp.h"
#include "job.h"
#include "libkcddb/cdinfodialog.h"
#include "prefs.h"
#include "kcompactdisc.h"
#include <QLabel>
#include <k3listview.h>
//Added by qt3to4:
#include <QPixmap>
#include <QKeyEvent>
#include <k3listview.h>
#include <QPushButton>
#include <QSpinBox>
#include <kmessagebox.h>
#include <kurl.h>
#include <QRegExp>
#include <kinputdialog.h>
#include <kprotocolinfo.h>

#include <kconfig.h>
#include <kapplication.h>

#include <QTimer>
#include <qfileinfo.h>
#include <kcombobox.h>
#include <kdebug.h>
#include <k3process.h>
#include <fixx11h.h>

using namespace KCDDB;

/**
 * Constructor, connect up slots and signals.
 */
TracksImp::TracksImp( QWidget* parent, const char* name) :
	Tracks(parent),
	cddbInfo() {
	setObjectName(name);
	cd = new KCompactDisc;

	connect(cd, SIGNAL(discChanged(unsigned)), this, SLOT(newDisc(unsigned)));

	connect(trackListing, SIGNAL(clicked( Q3ListViewItem * )), this, SLOT(selectTrack(Q3ListViewItem*)));
	connect(trackListing, SIGNAL(doubleClicked(Q3ListViewItem *)), this, SLOT(editInformation()));
	connect(trackListing, SIGNAL(returnPressed(Q3ListViewItem *)), this, SLOT(editInformation()));
	connect(selectAllTracksButton, SIGNAL(clicked()), this, SLOT(selectAllTracks()));
	connect(deselectAllTracksButton, SIGNAL(clicked()), this, SLOT(deselectAllTracks()));
	
	connect(deviceCombo, SIGNAL(textChanged(const QString &)), this, SLOT(changeDevice(const QString &)));
	
	selectAllTracksButton->setEnabled( false );
	deselectAllTracksButton->setEnabled( false );
	
	cddb = new KCDDB::Client();
	cddb->setBlockingMode(false);
	connect(cddb, SIGNAL(finished(KCDDB::Result)),
	                  this, SLOT(lookupCDDBDone(KCDDB::Result)));
	trackListing->setSorting(-1, false);
	loadSettings();
}

/**
 * store the current device from the combo.
 */
TracksImp::~TracksImp() {
	QStringList list;
	if( deviceCombo->count() != 0)
		list.append(deviceCombo->currentText());
	for ( int i=0; i<deviceCombo->count();i++ ) {
		QString text = deviceCombo->text(i);
		if( list.find(text) == list.end())
			list.append(text);
		if( list.count() == 5)
			break;
	}

	Prefs::setDevice(list);
	Prefs::self()->writeConfig();
}

/**
 * Load the class settings. 
 */
void TracksImp::loadSettings() {
	QStringList list;

	// Add the saved list, no dups
	QStringList prefsList = Prefs::device();
	QStringList::Iterator it;
	for ( it = prefsList.begin(); it != prefsList.end(); ++it ) {
		if( list.find( *it ) == list.end())
			list.append(*it);
	}
	// Get current list, no dups
	for ( int i=0; i<deviceCombo->count();i++ ) {
		QString text = deviceCombo->text(i);
		if( list.find(text) == list.end())
			list.append(text);
	}

	// Set list, get top one
	deviceCombo->clear();
	deviceCombo->addItems(list);

	changeDevice(deviceCombo->currentText());
}

void TracksImp::newDisc(unsigned tracks)
{
	if (!tracks)
	{
		kDebug(60002) << "newDisc - No disc" << endl;
		cddbInfo.clear();
		cddbInfo.set(Title, i18n("No disc"));
		newAlbum();
		emit(hasCD(false));

		selectAllTracksButton->setEnabled( false );
		deselectAllTracksButton->setEnabled( false );

		return;
	}
	
	unsigned discId = cd->discId();
	kDebug(60002) << "newDisc - " << discId << endl;
	emit(hasCD(true));

	selectAllTracksButton->setEnabled( true );
	deselectAllTracksButton->setEnabled( true );
 
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
bool TracksImp::hasCD(){
	return !cd->isNoDisc();
}

/**
 * The device text has changed.
 * @param file - the new text to check.
 */
void TracksImp::changeDevice(const QString &file ) {
	QString newDevice = KCompactDisc::urlToDevice(file);

	if( newDevice == cd->deviceUrl().url() ) {
		//qDebug("Device names match, returning");
		return;
	}

	QFileInfo fileInfo(newDevice);
	if( !fileInfo.exists() || fileInfo.isDir()) {
		//qDebug("Device file !exist or isDir or !file");
		return;
	}

	if (!cd->setDevice(newDevice, 50, false))
	{
		QString errstring =
		  i18n("CDROM read or access error (or no audio disk in drive).\n"\
		    "Please make sure you have access permissions to:\n%1",
		     file);
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
			uint c = 0;
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

/**
 * Bring up the dialog to edit the information about this album.
 * If there is not currently selected track return.
 * If ok is pressed then store the information and update track name.
 */
void TracksImp::editInformation( ) {
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
		cddb->store(cddbInfo);
	}
	delete dialog;
}

QString TracksImp::formatTime(unsigned ms)
{
	QTime time;

	time = time.addMSecs((int)ms);

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
void TracksImp::startSession( int encoder ) {
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

	foreach(TracksItem* item, selected)
	{
		Job *newJob = new Job();
		newJob->encoder = encoder;
		newJob->device = cd->deviceUrl().url();
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
	if( lastJob)
		lastJob->lastSongInAlbum = true;

	KMessageBox::information(this,
	i18n("%1 Job(s) have been started.  You can watch their progress in the "\
	   "jobs section.", selected.count()),
	i18n("Jobs have started"), i18n("Jobs have started"));
}

/**
 * Selects and unselects the tracks.
 * @param currentItem the track to swich the selection choice.
 */
void TracksImp::selectTrack( Q3ListViewItem *item ) {
	if( !item )
		return;

#define item static_cast<TracksItem*>(item)
	item->setChecked( !item->checked() );
#undef item
}

QList<TracksItem *> TracksImp::selectedTracks()
{
	QList<TracksItem *> selected;
	TracksItem *item = static_cast<TracksItem*>(trackListing->firstChild());

	while( item )
	{
		if( item->checked() )
			selected.append( item );
		item = static_cast<TracksItem*>(item->nextSibling());
	}
	return selected;
}

/**
 * Turn on all of the tracks.
 */
void TracksImp::selectAllTracks() {
	TracksItem *currentItem = static_cast<TracksItem*>(trackListing->firstChild());
	while( currentItem )
	{
		currentItem->setChecked( true );
		currentItem = static_cast<TracksItem*>(currentItem->nextSibling());
	}
}

/**
 * Turn off all of the tracks.
 */
void TracksImp::deselectAllTracks() {
	TracksItem *currentItem = static_cast<TracksItem*>(trackListing->firstChild());
	while( currentItem )
	{
		currentItem->setChecked( false );
		currentItem = static_cast<TracksItem*>(currentItem->nextSibling());
	}
}

/**
 * Set the current stats for the new album being displayed.
 */
void TracksImp::newAlbum() {
	QString albumText = cddbInfo.get(Title).toString();
	if( !cddbInfo.get(Artist).toString().isEmpty() )
		albumText = cddbInfo.get(Artist).toString() + i18n( " - " ) + albumText;

	albumName->setText( albumText );
	trackListing->clear();
	selectAllTracksButton->setEnabled(false);
	deselectAllTracksButton->setEnabled(false);
	emit(hasTracks(false));

	TracksItem *last = 0;
	for (unsigned i = 0; i < cddbInfo.numberOfTracks(); i++)
	{
		TrackInfo ti = cddbInfo.track(i);
		// There is a new track for this title.  Add it to the list of tracks.
		QString trackLength = formatTime(cd->trackLength(i+1));
		last = new TracksItem(trackListing, last, ti.get(Title).toString(), ti.get(Artist).toString(),
			i+1, trackLength, ti.get(Comment).toString());
	}

	if (cddbInfo.numberOfTracks())
	{
		// Set the current selected track to the first one.
		trackListing->setCurrentItem(trackListing->firstChild());
		selectAllTracksButton->setEnabled(true);
		deselectAllTracksButton->setEnabled(true);
		emit(hasTracks(true));
	}
}


/**
 * If the user presses the F2 key, trigger renaming of the title.
 * @param event the QKeyEvent passed to this event handler.
 */
void TracksImp::keyPressEvent(QKeyEvent *event) {
	Q3ListViewItem *item = trackListing->selectedItem();
	if( !item ) return;

	if( event->key() == Qt::Key_F2 )
	{
		item->setRenameEnabled( HEADER_TRACK_NAME, true );
		item->startRename( HEADER_TRACK_NAME );
		event->accept();
	}
	else
		Tracks::keyPressEvent(event);
}

/**
 * Eject the current cd device
 */
void TracksImp::eject() {
	ejectDevice(KCompactDisc::urlToDevice(cd->deviceUrl()));
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

