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
#include "tracksimp.h"
#include "job.h"
#include "infodialog.h"
#include "prefs.h"
#include <qlabel.h>
#include <qlistview.h>
#include <qpushbutton.h>
#include <qspinbox.h>
#include <kiconloader.h>
#include <kmessagebox.h>
#include <kurl.h>
#include <qregexp.h>
#include <kinputdialog.h>

#define HEADER_RIP 0
#define HEADER_TRACK 1
#define HEADER_LENGTH 2
#define HEADER_TRACK_NAME 3
#define HEADER_TRACK_ARTIST 4
#define HEADER_TRACK_COMMENT 5

#include "kaudiocreator_workman.h"

#include <kconfig.h>
#include <kapplication.h>

#include <qtimer.h>
#include <qfileinfo.h>
#include <kinputdialog.h>
#include <kcombobox.h>
#include <kdebug.h>
#include <kprocess.h>

/**
 * Constructor, connect up slots and signals.
 */
TracksImp::TracksImp( QWidget* parent, const char* name):Tracks(parent,name), CDid(0), album(""), group(""), genre(""), year(-1){
  connect(trackListing, SIGNAL(clicked( QListViewItem * )), this, SLOT(selectTrack(QListViewItem*))); 
  connect(trackListing, SIGNAL(doubleClicked(QListViewItem *)), this, SLOT(editInformation()));
  connect(trackListing, SIGNAL(returnPressed(QListViewItem *)), this, SLOT(editInformation()));
  connect(selectAllTracksButton, SIGNAL(clicked()), this, SLOT(selectAllTracks()));
  connect(deselectAllTracksButton, SIGNAL(clicked()), this, SLOT(deselectAllTracks()));
  
  connect(deviceCombo, SIGNAL(textChanged(const QString &)), this, SLOT(changeDevice(const QString &)));
  
  cddb = new KCDDB::Client();
  cddb->setBlockingMode(false);
  connect(cddb, SIGNAL(finished(CDDB::Result)),
          this, SLOT(cddbDone(CDDB::Result)));

  trackListing->setSorting(-1, false);
  // DON't i18n this!  It is to catch stupid people who don't bother setting the genre and
  // encoders that barf on empty strings for the genre (e.g. lame).
  genres.insert("Unknown", "Pop");
  genres.insert(i18n("A Cappella"), "A Cappella");
  genres.insert(i18n("Acid Jazz"), "Acid Jazz");
  genres.insert(i18n("Acid Punk"), "Acid Punk");
  genres.insert(i18n("Acid"), "Acid");
  genres.insert(i18n("Acoustic"), "Acoustic");
  genres.insert(i18n("Alternative"), "Alternative");
  genres.insert(i18n("Alt. Rock"), "Alt. Rock");
  genres.insert(i18n("Ambient"), "Ambient");
  genres.insert(i18n("Anime"), "Anime");
  genres.insert(i18n("Avantgarde"), "Avantgarde");
  genres.insert(i18n("Ballad"), "Ballad");
  genres.insert(i18n("Bass"), "Bass");
  genres.insert(i18n("Beat"), "Beat");
  genres.insert(i18n("Bebop"), "Bebop");
  genres.insert(i18n("Big Band"), "Big Band");
  genres.insert(i18n("Black Metal"), "Black Metal");
  genres.insert(i18n("Bluegrass"), "Bluegrass");
  genres.insert(i18n("Blues"), "Blues");
  genres.insert(i18n("Booty Bass"), "Booty Bass");
  genres.insert(i18n("BritPop"), "BritPop");
  genres.insert(i18n("Cabaret"), "Cabaret");
  genres.insert(i18n("Celtic"), "Celtic");
  genres.insert(i18n("Chamber Music"), "Chamber Music");
  genres.insert(i18n("Chanson"), "Chanson");
  genres.insert(i18n("Chorus"), "Chorus");
  genres.insert(i18n("Christian Gangsta Rap"), "Christian Gangsta Rap");
  genres.insert(i18n("Christian Rap"), "Christian Rap");
  genres.insert(i18n("Christian Rock"), "Christian Rock");
  genres.insert(i18n("Classical"), "Classical");
  genres.insert(i18n("Classic Rock"), "Classic Rock");
  genres.insert(i18n("Club-house"), "Club-house");
  genres.insert(i18n("Club"), "Club");
  genres.insert(i18n("Comedy"), "Comedy");
  genres.insert(i18n("Contemporary Christian"), "Contemporary Christian");
  genres.insert(i18n("Country"), "Country");
  genres.insert(i18n("Crossover"), "Crossover");
  genres.insert(i18n("Cult"), "Cult");
  genres.insert(i18n("Dance Hall"), "Dance Hall");
  genres.insert(i18n("Dance"), "Dance");
  genres.insert(i18n("Darkwave"), "Darkwave");
  genres.insert(i18n("Death Metal"), "Death Metal");
  genres.insert(i18n("Disco"), "Disco");
  genres.insert(i18n("Dream"), "Dream");
  genres.insert(i18n("Drum & Bass"), "Drum & Bass");
  genres.insert(i18n("Drum Solo"), "Drum Solo");
  genres.insert(i18n("Duet"), "Duet");
  genres.insert(i18n("Easy Listening"), "Easy Listening");
  genres.insert(i18n("Electronic"), "Electronic");
  genres.insert(i18n("Ethnic"), "Ethnic");
  genres.insert(i18n("Eurodance"), "Eurodance");
  genres.insert(i18n("Euro-House"), "Euro-House");
  genres.insert(i18n("Euro-Techno"), "Euro-Techno");
  genres.insert(i18n("Fast-Fusion"), "Fast-Fusion");
  genres.insert(i18n("Folklore"), "Folklore");
  genres.insert(i18n("Folk/Rock"), "Folk/Rock");
  genres.insert(i18n("Folk"), "Folk");
  genres.insert(i18n("Freestyle"), "Freestyle");
  genres.insert(i18n("Funk"), "Funk");
  genres.insert(i18n("Fusion"), "Fusion");
  genres.insert(i18n("Game"), "Game");
  genres.insert(i18n("Gangsta Rap"), "Gangsta Rap");
  genres.insert(i18n("Goa"), "Goa");
  genres.insert(i18n("Gospel"), "Gospel");
  genres.insert(i18n("Gothic Rock"), "Gothic Rock");
  genres.insert(i18n("Gothic"), "Gothic");
  genres.insert(i18n("Grunge"), "Grunge");
  genres.insert(i18n("Hardcore"), "Hardcore");
  genres.insert(i18n("Hard Rock"), "Hard Rock");
  genres.insert(i18n("Heavy Metal"), "Heavy Metal");
  genres.insert(i18n("Hip-Hop"), "Hip-Hop");
  genres.insert(i18n("House"), "House");
  genres.insert(i18n("Humor"), "Humor");
  genres.insert(i18n("Indie"), "Indie");
  genres.insert(i18n("Industrial"), "Industrial");
  genres.insert(i18n("Instrumental Pop"), "Instrumental Pop");
  genres.insert(i18n("Instrumental Rock"), "Instrumental Rock");
  genres.insert(i18n("Instrumental"), "Instrumental");
  genres.insert(i18n("Jazz+Funk"), "Jazz+Funk");
  genres.insert(i18n("Jazz"), "Jazz");
  genres.insert(i18n("JPop"), "JPop");
  genres.insert(i18n("Jungle"), "Jungle");
  genres.insert(i18n("Latin"), "Latin");
  genres.insert(i18n("Lo-Fi"), "Lo-Fi");
  genres.insert(i18n("Meditative"), "Meditative");
  genres.insert(i18n("Merengue"), "Merengue");
  genres.insert(i18n("Metal"), "Metal");
  genres.insert(i18n("Musical"), "Musical");
  genres.insert(i18n("National Folk"), "National Folk");
  genres.insert(i18n("Native American"), "Native American");
  genres.insert(i18n("Negerpunk"), "Negerpunk");
  genres.insert(i18n("New Age"), "New Age");
  genres.insert(i18n("New Wave"), "New Wave");
  genres.insert(i18n("Noise"), "Noise");
  genres.insert(i18n("Oldies"), "Oldies");
  genres.insert(i18n("Opera"), "Opera");
  genres.insert(i18n("Other"), "Other");
  genres.insert(i18n("Polka"), "Polka");
  genres.insert(i18n("Polsk Punk"), "Polsk Punk");
  genres.insert(i18n("Pop-Funk"), "Pop-Funk");
  genres.insert(i18n("Pop/Funk"), "Pop/Funk");
  genres.insert(i18n("Pop"), "Pop");
  genres.insert(i18n("Porn Groove"), "Porn Groove");
  genres.insert(i18n("Power Ballad"), "Power Ballad");
  genres.insert(i18n("Pranks"), "Pranks");
  genres.insert(i18n("Primus"), "Primus");
  genres.insert(i18n("Progressive Rock"), "Progressive Rock");
  genres.insert(i18n("Psychedelic Rock"), "Psychedelic Rock");
  genres.insert(i18n("Psychedelic"), "Psychedelic");
  genres.insert(i18n("Punk Rock"), "Punk Rock");
  genres.insert(i18n("Punk"), "Punk");
  genres.insert(i18n("R&B"), "R&B");
  genres.insert(i18n("Rap"), "Rap");
  genres.insert(i18n("Rave"), "Rave");
  genres.insert(i18n("Reggae"), "Reggae");
  genres.insert(i18n("Retro"), "Retro");
  genres.insert(i18n("Revival"), "Revival");
  genres.insert(i18n("Rhythmic Soul"), "Rhythmic Soul");
  genres.insert(i18n("Rock & Roll"), "Rock & Roll");
  genres.insert(i18n("Rock"), "Rock");
  genres.insert(i18n("Salsa"), "Salsa");
  genres.insert(i18n("Samba"), "Samba");
  genres.insert(i18n("Satire"), "Satire");
  genres.insert(i18n("Showtunes"), "Showtunes");
  genres.insert(i18n("Ska"), "Ska");
  genres.insert(i18n("Slow Jam"), "Slow Jam");
  genres.insert(i18n("Slow Rock"), "Slow Rock");
  genres.insert(i18n("Sonata"), "Sonata");
  genres.insert(i18n("Soul"), "Soul");
  genres.insert(i18n("Sound Clip"), "Sound Clip");
  genres.insert(i18n("Soundtrack"), "Soundtrack");
  genres.insert(i18n("Southern Rock"), "Southern Rock");
  genres.insert(i18n("Space"), "Space");
  genres.insert(i18n("Speech"), "Speech");
  genres.insert(i18n("Swing"), "Swing");
  genres.insert(i18n("Symphonic Rock"), "Symphonic Rock");
  genres.insert(i18n("Symphony"), "Symphony");
  genres.insert(i18n("Synthpop"), "Synthpop");
  genres.insert(i18n("Tango"), "Tango");
  genres.insert(i18n("Techno-Industrial"), "Techno-Industrial");
  genres.insert(i18n("Techno"), "Techno");
  genres.insert(i18n("Terror"), "Terror");
  genres.insert(i18n("Thrash Metal"), "Thrash Metal");
  genres.insert(i18n("Top 40"), "Top 40");
  genres.insert(i18n("Trailer"), "Trailer");
  genres.insert(i18n("Trance"), "Trance");
  genres.insert(i18n("Tribal"), "Tribal");
  genres.insert(i18n("Trip-Hop"), "Trip-Hop");
  genres.insert(i18n("Vocal"), "Vocal");
  loadSettings();
  QTimer *timer = new QTimer( this );
  connect( timer, SIGNAL(timeout()), this, SLOT(timerDone()) );
  timer->start( 1500, false ); // 1.5 seconds forever timer
}

/**
 * store the current device from the combo.
 */
TracksImp::~TracksImp(){
  QStringList list;
  for(int i=0; i<deviceCombo->count();i++){
    QString text = deviceCombo->text(i);
    if(list.find(text) == list.end())
      list.append(text);
    if(list.count() == 5)
      break;
  }

  Prefs::setDevice(list);
  Prefs::writeConfig();
}

/**
 * Load the class settings. 
 */ 
void TracksImp::loadSettings(){
  // Get current list, no dups
  QStringList list;
  for(int i=0; i<deviceCombo->count();i++){
    QString text = deviceCombo->text(i);
    if(list.find(text) == list.end())
      list.append(text);
  }
  // Add the saved list, no dups
  QStringList prefsList = Prefs::device();
  for ( QStringList::Iterator it = prefsList.begin(); it != prefsList.end(); ++it ) {
    if(list.find( *it ) == list.end())
      list.append(*it);
  }
  
  // Set list, get top one
  deviceCombo->clear();
  deviceCombo->insertStringList(list);
  device = deviceCombo->currentText();
}

/**
 * Check for changes in the cd.
 */ 
void TracksImp::timerDone(){
  int status = wm_cd_init( WM_CDIN, (char *)qstrdup(QFile::encodeName(device)), NULL, NULL, NULL);
  if(status == dstatus){
    wm_cd_destroy();
    return;
  }
  kdDebug(60002) << "Drive initialization return status: " << status << endl;
  dstatus = status;
  
  if(WM_CDS_NO_DISC(status)){
    kdDebug(60002) << "No disk." << endl;
    emit(hasCD(false));
    newAlbum();
    CDid = 0;
    wm_cd_destroy();
    return;
  }
  
  if(status < 0) {
    QString errstring =
               i18n("CDROM read or access error (or no audio disc in drive).\n"\
                    "Please make sure you have access permissions to:\n%1")
               .arg(device);
    KMessageBox::error(this, errstring, i18n("Error"));
    wm_cd_destroy();
    return;
  }

  unsigned long currentDistID = cddb_discid();
  if(currentDistID == CDid){
    wm_cd_destroy();
    return;
  }
  
  // A new album
  newAlbum();
  emit(hasCD(true));
  CDid = currentDistID;
  kdDebug(60002) << "New disk.  Disk id: " << CDid << endl;
  int numberOfTracks = wm_cd_getcountoftracks();
  for(int i=numberOfTracks; i>0; i--){
      newSong(i, QString::number(i).rightJustify(2, '0'), (cd->trk[i-1]).length, "");
  }
  if(Prefs::performCDDBauto())
    cddbCD();

  wm_cd_destroy();
}

/**
 * The device text has changed.  If valid different file from device
 * then call timerDone() to re-initialize the cd library and check its status.
 * @param file - the new text to check.
 */ 
void TracksImp::changeDevice(const QString &file){
  if(file == device){
    //qDebug("Device names match, returning");
    return;
  }
  
  QFileInfo fileInfo(file);
  if(!fileInfo.exists() || fileInfo.isDir()){
    //qDebug("Device file !exist or isDir or !file");
    return;
  }
 
  device = file;
  
  KApplication::setOverrideCursor(Qt::waitCursor);
  timerDone();
  KApplication::restoreOverrideCursor();
}

/**
 * Helper function (toolbar button) for users.
 **/ 
void TracksImp::performCDDB(){
  int status = wm_cd_init( WM_CDIN, (char *)qstrdup(QFile::encodeName(device)), NULL, NULL, NULL);
  if(WM_CDS_NO_DISC(status)){
    KMessageBox::sorry(this, i18n("Please insert a disk."), i18n("CDDB Failed"));
    wm_cd_destroy();
    return;
  }

  cddbCD();
  
  wm_cd_destroy();
}

/**
 * See if we can't get the cddb value for this cd.
 * wm_cd_init must be called before this.
 * @return true if successful.
 */ 
void TracksImp::cddbCD(){
  KCDDB::TrackOffsetList qvl;

  int numberOfTracks = wm_cd_getcountoftracks();
  for(int i=0; i<numberOfTracks; i++){
    qvl.append((cd->trk[i]).start);
    //kdDebug(60002) << "Track: " << i << (cd->trk[i]).start << endl;
  }

  qvl.append((cd->trk[0]).start);
  //kdDebug(60002) << (cd->trk[0]).start << endl;
  qvl.append((cd->trk[numberOfTracks]).start );
  //kdDebug(60002) << (cd->trk[numberOfTracks]).start << endl;
  
  cddb->lookup(qvl);
}

/**
 * The non blocking CDDB function calling has finished.  Report an error or
 * continue.
 * @param result the success or failure of the cddb retrieval.
 */
void TracksImp::cddbDone(CDDB::Result result){
  // CDDBTODO: figure out why using CDDB::Success doesn't compile?!
  if ((result != 0 /*KCDDB::CDDB::Lookup::Success*/) &&
      (result != KCDDB::CDDB::MultipleRecordFound))
  {
     KMessageBox::sorry(this, i18n("Unable to retrieve CDDB information."), i18n("CDDB Failed"));
    return;
  }

  // Choose the cddb entry
  KCDDB::CDInfo info = cddb->bestLookupResponse();
  // TODO Why doesn't libcddb not return MultipleRecordFound?
  //if(result == KCDDB::CDDB::MultipleRecordFound){
  if(Prefs::promptIfIncompleteInfo() && cddb->lookupResponse().count() > 1){
    CDInfoList cddb_info = cddb->lookupResponse();
    CDInfoList::iterator it;
    QStringList list;
    uint defaultChoice = 0;
    uint maxrev = 0;
    uint c = 0;
    for ( it = cddb_info.begin(); it != cddb_info.end(); ++it ){
      list.append( QString("%1, %2, %3").arg((*it).artist).arg((*it).title).arg((*it).genre));
      KCDDB::CDInfo cinfo = *it;
      if ( ( *it ).revision >= maxrev ) {
        maxrev = info.revision;
        defaultChoice = c;
      }
      c++;
    }
 
    bool ok(false); 
    QString res = KInputDialog::getItem(
            i18n("Select  a CDDB entry - KAudioCreator"), i18n("Select a CDDB entry:"), list, defaultChoice, false, &ok,
            this );
    if ( ok ) {
      // The user selected and item and pressed OK
      uint c = 0;
      for ( QStringList::Iterator it = list.begin(); it != list.end(); ++it ) {
        if(*it == res)  break;
        c++;
      }
      if(cddb_info.size() <= c)
        info = cddb_info[c];
    } else {
      // user pressed Cancel
    }
  }

  // Fill in all album data
  newAlbum(info.artist, info.title, info.year, info.genre, info.revision, info.category, info.extd);  
      
  KCDDB::TrackInfoList t = info.trackInfoList;
  for (unsigned i = t.count(); i > 0; i--)
  {
    if(cd->trk[i-1].data == 0)
    {
      QString n;
      n.sprintf("%02d ", i-1 + 1);
      newSong(i, (n + t[i-1].title), cd->trk[i-1].length, t[i-1].extt);
    }
  }

  // See if the user wishes to automaticly rip after successfully retrieving
  if(Prefs::autoRip())
    ripWholeAlbum();
}

/**
 * Bring up the dialog to edit the information about this album.
 * If there is not currently selected track return.
 * If ok is pressed then store the information and update track name.
 */
void TracksImp::editInformation(){
  QListViewItem * currentItem = trackListing->currentItem();
  if( currentItem == 0 ){
    KMessageBox::sorry(this, i18n("Please select a track."), i18n("No Track Selected"));
    return;
  }

  // Create dialog.
  dialog = new AlbumEditor(this, "Album info editor dialog", true);
  dialog->artist->setText(group);
  dialog->album->setText(album);
  dialog->year->setValue(year);
  dialog->comment->setText(comment);
  dialog->trackLabel->setText(i18n("Track %1").arg(currentItem->text(HEADER_TRACK)));
  dialog->track_title->setText(currentItem->text(HEADER_TRACK_NAME));
  dialog->track_artist->setText(currentItem->text(HEADER_TRACK_ARTIST));
  dialog->track_comment->setText(currentItem->text(HEADER_TRACK_COMMENT));
  dialog->genre->insertStringList(genres.keys());
  int totalGenres = dialog->genre->count();
  if(genre.isEmpty())
    genre = i18n("Other");
  
  for(int i = 0; i < totalGenres; i++){
    if(dialog->genre->text(i) == genre){
      dialog->genre->setCurrentItem(i);
      break;
    }
  }

  // set focus to track title
  dialog->track_title->setFocus();
  dialog->track_title->selectAll();

  connect(dialog->buttonPrevious, SIGNAL(clicked()),
    this, SLOT(editPreviousTrack()));
  connect(dialog->buttonNext, SIGNAL(clicked()),
    this, SLOT(editNextTrack()));

  // Show dialog->and save results.
  bool okClicked = dialog->exec();
  if(okClicked){
    trackListing->currentItem()->setText(HEADER_TRACK_NAME, dialog->track_title->text());
    trackListing->currentItem()->setText(HEADER_TRACK_ARTIST, dialog->track_artist->text());
    trackListing->currentItem()->setText(HEADER_TRACK_COMMENT, dialog->track_comment->text());
    
    if( group != dialog->artist->text()){
      int r = KMessageBox::questionYesNo(this, i18n("You have changed the album artist. Would you like all of the track artists that had the old name to be changed to the new name?"), i18n("Album Artist Changed"));
      if( r == KMessageBox::Yes ){
        QListViewItem * currentItem = trackListing->firstChild();
        while( currentItem != 0 ){
          if(group == currentItem->text(HEADER_TRACK_ARTIST))
            currentItem->setText(HEADER_TRACK_ARTIST, dialog->artist->text());
          currentItem = currentItem->nextSibling();
        }
      }
      group = dialog->artist->text();
    }
    album = dialog->album->text();
    year = dialog->year->value();
    comment = dialog->comment->text();
    genre = dialog->genre->currentText();

    QString newTitle = QString("%1 - %2").arg(group).arg(album);
    if(albumName->text() != newTitle)
      albumName->setText(newTitle);

    KCDDB::CDInfo info;
    setCdInfo(info);

    if (!info.category.isEmpty())
      KCDDB::Cache::store(info);
  }
  delete dialog;
}

/**
 * Helper function.  
 * Selects all tracks and then calls startSession to rip them all.
 */
void TracksImp::ripWholeAlbum(){
  selectAllTracks();
  startSession();
}

void TracksImp::setCdInfo(KCDDB::CDInfo &info)
{
  info.artist = group;
  info.title = album;
  info.genre = genre;
  if (category.isEmpty())
  {
    QStringList catlist;
    catlist << "blues" << "classical" << "country"
	    << "data" << "folk" << "jazz" << "misc" << "newage" << "reggae"
	    << "rock" << "soundtrack";

    bool ok;

    category = KInputDialog::getItem( i18n( "Select Album Category" ),
        i18n( "Select a category for this album:" ), catlist, 0,
        false, &ok, this );

    if ( !ok )
      return;
  }
  info.category = category;
  info.id = QString::number(CDid, 16);
  info.extd = comment;
  info.year = year;
  info.length = ((cd->trk[wm_cd_getcountoftracks()]).start - (cd->trk[0]).start) / 75;
  info.revision = revision;

  info.trackInfoList.clear();
  QListViewItem * currentItem = trackListing->firstChild();
  while( currentItem != 0 )
  {
    KCDDB::TrackInfo t;
    t.title = currentItem->text(HEADER_TRACK_NAME);
    t.extt = currentItem->text(HEADER_TRACK_COMMENT);

    info.trackInfoList.append(t);

    currentItem = currentItem->nextSibling();
  }
}

/**
 * Start of the "ripping session" by emiting signals to rip the selected tracks.
 * If any album information is not set, notify the user first.
 */
void TracksImp::startSession(){
  if(trackListing->childCount() == 0){
    KMessageBox:: sorry(this, i18n("No tracks are selected to rip. Please select at least 1 track before ripping."), i18n("No Tracks Selected"));
    return;
  }

  QStringList list;
  if( genre == "Unknown" )
    list += "Genre";
  if( year == 0 )
    list += "Year";
  if( group == "Unknown Artist")
    list += "Artist";
  if( album == "Unknown Album")
    list += "Album";
  
  if( Prefs::promptIfIncompleteInfo() && list.count()>0 ){
    int r = KMessageBox::questionYesNo(this, i18n("Part of the album is not set: %1.\n (To change album information click the \"Edit Information\" button.)\n Would you like to rip the selected tracks anyway?").arg(list.join(", ")), i18n("Album Information Incomplete"));
    if( r == KMessageBox::No )
      return;
  }
  QListViewItem * currentItem = trackListing->firstChild();
  Job *lastJob = NULL;
  int counter = 0;
  while( currentItem != 0 ){
    if(currentItem->pixmap(HEADER_RIP) != NULL ){
      Job *newJob = new Job();
      newJob->device = device;
      newJob->album = album;
      newJob->genre = genres[genre];
      if(newJob->genre.isEmpty())
        newJob->genre = "Pop";
      newJob->group = group;
      newJob->comment = comment;
      newJob->year = year;
      newJob->track = currentItem->text(HEADER_TRACK).toInt();
      
      newJob->track_title = currentItem->text(HEADER_TRACK_NAME);
      newJob->track_artist = currentItem->text(HEADER_TRACK_ARTIST);
      newJob->track_comment = currentItem->text(HEADER_TRACK_COMMENT);
      lastJob = newJob;
      emit( ripTrack(newJob) ); 
      counter++;
    }
    currentItem = currentItem->nextSibling();
  }
  if(lastJob)
    lastJob->lastSongInAlbum = true;

  if(counter == 0){
    KMessageBox:: sorry(this, i18n("No tracks are selected to rip. Please select at least 1 track before ripping."), i18n("No Tracks Selected"));
    return;
  }

  KMessageBox::information(this,
  i18n("%1 Job(s) have been started.  You can watch their progress in the jobs section.").arg(counter),
 i18n("Jobs have started"), i18n("Jobs have started"));
}

/**
 * Selects and unselects the tracks.
 * @param currentItem the track to swich the selection choice.
 */
void TracksImp::selectTrack(QListViewItem *currentItem){
  if(!currentItem)
    return;
  if(currentItem->pixmap(HEADER_RIP) != NULL){
    QPixmap empty;
    currentItem->setPixmap(HEADER_RIP, empty);
  }
  else
    currentItem->setPixmap(HEADER_RIP, SmallIcon("check", currentItem->height()-2));
}

/**
 * Turn on all of the tracks.
 */
void TracksImp::selectAllTracks(){
  QListViewItem *currentItem = trackListing->firstChild();
  while( currentItem != 0 ){
    currentItem->setPixmap(HEADER_RIP, SmallIcon("check", currentItem->height()-2));
    currentItem = currentItem->nextSibling();
  }
}

/**
 * Turn off all of the tracks.
 */
void TracksImp::deselectAllTracks(){
  QListViewItem *currentItem = trackListing->firstChild();
  QPixmap empty;
  while( currentItem != 0 ){
    currentItem->setPixmap(HEADER_RIP, empty);
    currentItem = currentItem->nextSibling();
  }
}

/**
 * Set the current stats for the new album being displayed.
 */
void TracksImp::newAlbum(const QString &newGroup, const QString &newAlbum,
			uint newYear, const QString &newGenre, uint newRevision,
			const QString& newCategory, const QString& newComment){
  albumName->setText(QString("%1 - %2").arg(newGroup).arg(newAlbum));
  trackListing->clear();
  album = newAlbum;
  group = newGroup;
  year = newYear;
  genre = newGenre;
  revision = newRevision;
  category = newCategory;
  comment = newComment;


  selectAllTracksButton->setEnabled(false);
  deselectAllTracksButton->setEnabled(false);
  emit(hasTracks(false));
}

/**
 * There is a new track for this album.  Add it to the list of tracks.  Set the
 * current selected track to the first one.
 * @param track the track number.
 * @param title the name of the track.
 * @param length the lenght of track.
 */
void TracksImp::newSong(int track, const QString &newTitle, int length, const QString &comment){
  QString title = newTitle.mid(newTitle.find(' ',0)+1);
  title = KURL::decode_string(title);
  title.replace(QRegExp("/"), "-");
  QString trackLength = QString("%1:%2%3").arg(length/60).arg((length % 60)/10).arg((length % 60)%10);
  QListViewItem * newItem = new QListViewItem(trackListing, "", QString("%1").arg(track), trackLength, title, group, comment);
  newItem->setRenameEnabled(HEADER_TRACK_NAME, TRUE);
  trackListing->setCurrentItem(trackListing->firstChild());

  selectAllTracksButton->setEnabled(true);
  deselectAllTracksButton->setEnabled(true);
  emit(hasTracks(true));
}

/**
 * If the user presses the F2 key, trigger renaming of the title.
 * @param event the QKeyEvent passed to this event handler.
 */
void TracksImp::keyPressEvent(QKeyEvent *event){
  if( trackListing->selectedItem() != NULL && event->key() == Qt::Key_F2 ) {
    event->accept();
    trackListing->selectedItem()->startRename(HEADER_TRACK_NAME);
  }
  else
    Tracks::keyPressEvent(event);
}

/**
 * Edit a differnt track
 */
void TracksImp::editOtherTrack(bool nextOneUp){
  QListViewItem* currentItem = trackListing->currentItem();
  if(!currentItem)
    return;
  currentItem->setText(HEADER_TRACK_NAME, dialog->track_title->text());
  currentItem->setText(HEADER_TRACK_ARTIST, dialog->track_artist->text());
  currentItem->setText(HEADER_TRACK_COMMENT, dialog->track_comment->text());
  QListViewItem* newCurrentItem;
  if(nextOneUp)
    newCurrentItem = currentItem->itemAbove();
  else
    newCurrentItem = currentItem->itemBelow();
  
  if (newCurrentItem)
  {
    trackListing->setCurrentItem(newCurrentItem);
    dialog->track_title->setText(newCurrentItem->text(HEADER_TRACK_NAME));
    dialog->track_artist->setText(newCurrentItem->text(HEADER_TRACK_ARTIST));
    dialog->track_comment->setText(newCurrentItem->text(HEADER_TRACK_COMMENT));
    dialog->trackLabel->setText(i18n("Track %1").arg(newCurrentItem->text(HEADER_TRACK)));
  }
  dialog->track_title->setFocus();
  dialog->track_title->selectAll();
}

void TracksImp::editNextTrack() {
  editOtherTrack(false);
}

void TracksImp::editPreviousTrack() {
  editOtherTrack(true);
}

/**
 * Eject the current cd device
 */
void TracksImp::eject(){
  ejectDevice(device);
}

/**
 * Eject a device
 * @param deviceToEject the device to eject.
 */
void TracksImp::ejectDevice(const QString &deviceToEject){
 KProcess *proc = new KProcess();
#ifdef __FreeBSD__
  *proc << "cdcontrol" << "-f" << deviceToEject << "eject";
#else
   *proc << "eject" << deviceToEject;
#endif
  connect(proc, SIGNAL(processExited(KProcess *)), this, SLOT(ejectDone(KProcess *)));
  proc->start(KProcess::NotifyOnExit,  KShellProcess::NoCommunication);
}
  
/**
 * When it is done ejecting, report any errors.
 * @param proc pointer to the process that ended.
 */ 
void TracksImp::ejectDone(KProcess *proc){
  int returnValue = proc->exitStatus();
  if(returnValue == 127){
    KMessageBox:: sorry(this, i18n("\"eject\" command not installed."), i18n("Cannot Eject"));
    return;
  }
  if(returnValue != 0){
    kdDebug(60002) << "Eject failed and returned: " << returnValue << endl;
    KMessageBox:: sorry(this, i18n("\"eject\" command failed."), i18n("Cannot Eject"));
    return;
  }
  delete proc;
}

#include "tracksimp.moc"

