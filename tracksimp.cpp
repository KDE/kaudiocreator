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


#include <dcopref.h>
#include <fixx11h.h>

#include <kapplication.h>
#include <kconfig.h>
#include <kcombobox.h>
#include <kdebug.h>
#include <kinputdialog.h>
#include <klistview.h>
#include <kprocess.h>
#include <kmessagebox.h>
#include <kurl.h>

#include <qfileinfo.h>
#include <qlabel.h>
#include <qptrlist.h>
#include <qpushbutton.h>
#include <qregexp.h>
#include <qspinbox.h>
#include <qtimer.h>

#include "job.h"
#include "kcompactdisc.h"
#include "libkcddb/cdinfodialogbase.h"
#include "prefs.h"
#include "tracksimp.h"

/**
 * Constructor, connect up slots and signals.
 */
TracksImp::TracksImp( QWidget* parent, const char* name) :
    Tracks(parent,name),
    cddbInfo() 
{
    cd = new KCompactDisc;

    connect(cd,SIGNAL(discChanged(unsigned)),this,SLOT(newDisc(unsigned)));

    connect(trackListing, SIGNAL(clicked( QListViewItem * )), this, SLOT(selectTrack(QListViewItem*)));
    connect(trackListing, SIGNAL(doubleClicked(QListViewItem *)), this, SLOT(editInformation()));
    connect(trackListing, SIGNAL(returnPressed(QListViewItem *)), this, SLOT(editInformation()));
    connect(selectAllTracksButton, SIGNAL(clicked()), this, SLOT(selectAllTracks()));
    connect(deselectAllTracksButton, SIGNAL(clicked()), this, SLOT(deselectAllTracks()));

    connect(deviceCombo, SIGNAL(textChanged(const QString &)), this, SLOT(changeDevice(const QString &)));

    selectAllTracksButton->setEnabled( false );
    deselectAllTracksButton->setEnabled( false );

    cddb = new KCDDB::Client();
    cddb->setBlockingMode(false);
    connect(cddb, SIGNAL(finished(CDDB::Result)), this, SLOT(lookupCDDBDone(CDDB::Result)));
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
    Prefs::writeConfig();
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
    deviceCombo->insertStringList(list);

    changeDevice(deviceCombo->currentText());
}

void TracksImp::newDisc(unsigned discId)
{
    if (discId == KCompactDisc::missingDisc)
    {
        kdDebug() << "newDisc - No disc" << endl;
        cddbInfo.clear();
        cddbInfo.title = i18n("No disc");
        newAlbum();
        emit(hasCD(false));

        selectAllTracksButton->setEnabled( false );
        deselectAllTracksButton->setEnabled( false );

        return;
    }

    kdDebug() << "newDisc - " << discId << endl;
    emit(hasCD(true));

    selectAllTracksButton->setEnabled( true );
    deselectAllTracksButton->setEnabled( true );

    cddbInfo.clear();

    cddbInfo.id = QString::number(discId, 16).rightJustify(8,'0');
    cddbInfo.length = cd->discLength();

    cddbInfo.artist = cd->discArtist();
    cddbInfo.title = cd->discTitle();

    // If it's a sampler, we'll do artist/title.
    bool isSampler = (cddbInfo.title.compare("Various") == 0);
    KCDDB::TrackInfo track;
    for (unsigned i = 1; i <= cd->tracks(); i++) {
        if (isSampler)
            track.title = cd->trackArtist(i) + " / " + cd->trackTitle(i);
        else
            track.title = cd->trackTitle(i);
        cddbInfo.trackInfoList.append(track);
    }

    newAlbum();

    if (Prefs::performCDDBauto())
        lookupCDDB();
}

/**
 * @return if there is a cd inserted or not.
 */
bool TracksImp::hasCD(){
    return cd->discId() != KCompactDisc::missingDisc;
}

/**
 * The device text has changed.
 * @param file - the new text to check.
 */
void TracksImp::changeDevice(const QString &file ) {
    QString newDevice = KCompactDisc::urlToDevice(file);

    if( newDevice == cd->device() ) {
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
            "Please make sure you have access permissions to:\n%1")
            .arg(file);
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
 * The non blocking CDDB function calling has finished.    Report an error or
 * continue.
 * @param result the success or failure of the cddb retrieval.
 */
void TracksImp::lookupCDDBDone(CDDB::Result result ) {
    if ((result != KCDDB::CDDB::Success) &&
        (result != KCDDB::CDDB::MultipleRecordFound))
    {
        KMessageBox::sorry(this, i18n("Unable to retrieve CDDB information."), i18n("CDDB Failed"));
        return;
    }

    // Choose the cddb entry
    KCDDB::CDInfo info = cddb->bestLookupResponse();
    // TODO Why doesn't libcddb not return MultipleRecordFound?
    //if( result == KCDDB::CDDB::MultipleRecordFound ) {
    if( Prefs::promptIfIncompleteInfo() && cddb->lookupResponse().count() > 1 ) {
        QString searchedCDId = cddbInfo.id;
        CDInfoList cddb_info = cddb->lookupResponse();
        CDInfoList::iterator it;
        QStringList list;
        for ( it = cddb_info.begin(); it != cddb_info.end(); ++it  ) {
            list.append( QString("%1, %2, %3").arg((*it).artist).arg((*it).title)
              .arg((*it).genre));
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
                if( *it == res)    break;
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
        if ( searchedCDId != cddbInfo.id )
            return;
    }

    // Some sanity provisions to ensure that the number of records matches what
    // the CD actually contains.
    while (info.trackInfoList.count() < cddbInfo.trackInfoList.count())
    {
        info.trackInfoList.append(KCDDB::TrackInfo());
    }
    while (info.trackInfoList.count() > cddbInfo.trackInfoList.count())
    {
        info.trackInfoList.pop_back();
    }
    cddbInfo = info;
    newAlbum();

    // See if the user wishes to automaticly rip after successfully retrieving
    if( Prefs::autoRip())
        ripWholeAlbum();
}

/**
 * Bring up the dialog to edit the information about this album.
 * If there is not currently selected track return.
 * If ok is pressed then store the information and update track name.
 */
void TracksImp::editInformation( )
{
    if( !hasCD() ) return;
    // Create dialog.
    KDialogBase *dialog = new KDialogBase( this, "name", false, i18n( "CD Editor" ),
                                           KDialogBase::Ok|KDialogBase::Cancel, KDialogBase::Ok, true );
    CDInfoDialogBase *base = new CDInfoDialogBase(dialog, "Album info editor dialog");
    // Workaround the fact that CDInfoDialogBase doesn't take
    // a const TrackOffsetList
    QValueList<unsigned> discSig = cd->discSignature();
    base->setInfo(cddbInfo, discSig);
    dialog->setMainWidget(base);

    // Show dialog->and save results.
    bool okClicked = dialog->exec();
    if( okClicked ) {
        cddbInfo = base->info();
        newAlbum();
        KCDDB::Cache::store(cddbInfo);
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
void TracksImp::startSession( int encoder )
{
    QPtrList<TracksItem> selected = selectedTracks();

    if( selected.isEmpty() )
    {
        int i = KMessageBox::questionYesNo( this, i18n("No tracks have been selected.  Would you like to rip the entire CD?"),
                                            i18n("No Tracks Selected"), i18n("Rip CD"), KStdGuiItem::cancel() );
        if( i == KMessageBox::No )
            return;

        selectAllTracks();
        selected = selectedTracks();
    }

    QStringList list;
    if( cddbInfo.genre == "Unknown" )
        list += "Genre";
    if( cddbInfo.year == 0 )
        list += "Year";
    if( cddbInfo.artist == "Unknown Artist")
        list += "Artist";
    if( cddbInfo.title == "Unknown Album")
        list += "Album";

    if( Prefs::promptIfIncompleteInfo() && list.count() > 0 )
    {
        int r = KMessageBox::questionYesNo( this, 
                 i18n( "Part of the album is not set: %1.\n (To change album information click the \"Edit Information\" button.)\n Would you like to rip the selected tracks anyway?").arg(list.join(", ")), i18n("Album Information Incomplete"), i18n("Rip"), KStdGuiItem::cancel() );

        if( r == KMessageBox::No )
            return;
    }

    if ( encoder == -1 )
        encoder = Prefs::currentEncoder();

    Job *lastJob = 0;
    TracksItem *item = selected.first();

    for( ; item ; item = selected.next() )
    {
        Job *newJob = new Job();
        newJob->encoder = encoder;
        newJob->device  = cd->device();
        newJob->album   = cddbInfo.title;
        newJob->genre   = cddbInfo.genre;
        if( newJob->genre.isEmpty() )
            newJob->genre = "Pop";

        newJob->group   = cddbInfo.artist;
        newJob->comment = cddbInfo.extd;
        newJob->year    = cddbInfo.year;
        newJob->track   = item->track();

//         newJob->track_title   = item->title();
        newJob->track_title   = item->text( HEADER_TRACK_NAME );
        newJob->track_artist  = item->artist();
        newJob->track_comment = item->comment();
        lastJob = newJob;
        emit( ripTrack(newJob) );
    }
    if( lastJob)
        lastJob->lastSongInAlbum = true;

    KMessageBox::information(this,
    i18n("%1 Job(s) have been started.  You can watch their progress in the "
         "jobs section.").arg( selected.count() ),
    i18n("Jobs have started"), i18n("Jobs have started"));
}

/**
 * Selects and unselects the tracks.
 * @param currentItem the track to swich the selection choice.
 */
void TracksImp::selectTrack( QListViewItem *item )
{
    if( !item )
        return;

#define item static_cast<TracksItem*>(item)
    item->setChecked( !item->checked() );
#undef item
}

QPtrList<TracksItem> TracksImp::selectedTracks()
{
    QPtrList<TracksItem> selected;
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
void TracksImp::selectAllTracks()
{
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
void TracksImp::deselectAllTracks()
{
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
void TracksImp::newAlbum()
{
    QString albumText = cddbInfo.title;
    if( !cddbInfo.artist.isEmpty() )
        albumText = cddbInfo.artist + i18n( " - " ) + albumText;

    albumName->setText( albumText );
    trackListing->clear();
    selectAllTracksButton->setEnabled(false);
    deselectAllTracksButton->setEnabled(false);

    emit( hasTracks(false) );

    KCDDB::TrackInfoList t = cddbInfo.trackInfoList;

    bool isSampler = true;
    for( unsigned i = 0; i < t.count(); i++ )
    {
        if (t[i].title.find(" / ") == -1)
        {
            isSampler = false;
            break;
        }
    }

    TracksItem *last = 0;
    for( unsigned i = 0; i < t.count(); i++ )
    {
        QString trackArtist;
        QString title;

        if( isSampler )
        {
            // Support for multiple artists stripping.
            int delimiter = t[i].title.find(" / ");
            Q_ASSERT( delimiter != -1 );
            trackArtist = t[i].title.left(delimiter);
            title = t[i].title.mid(delimiter + 3);
        }
        else {
            trackArtist = cddbInfo.artist;
            title = t[i].title;
        }

        // There is a new track for this title.  Add it to the list of tracks.
        QString trackLength = formatTime(cd->trackLength(i+1));
        last = new TracksItem( trackListing, last, title, trackArtist, i+1, trackLength, t[i].extt );
    }

    if( t.count() )
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
void TracksImp::keyPressEvent(QKeyEvent *event)
{
    QListViewItem *item = trackListing->selectedItem();
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
    ejectDevice(cd->device());
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

