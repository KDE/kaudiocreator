#include "tracksconfigimp.h"
#include "job.h"
#include "id3tagdialog.h"
#include <qlabel.h>
#include <qlistview.h>
#include <qpushbutton.h>
#include <qlineedit.h>
#include <qspinbox.h>
#include <qcombobox.h>
#include <qpixmap.h>
#include <kiconloader.h>
#include <kmessagebox.h>
#include <klocale.h>
#include <kurl.h>
#include <qregexp.h>

#define HEADER_TRACK 1
#define HEADER_NAME 3
#define HEADER_LENGTH 2
#define HEADER_RIP 0

/**
 * Constructor, connect up slots and signals.
 */
TracksConfigImp::TracksConfigImp( QWidget* parent, const char* name):TracksConfig(parent,name), album(""), group(""), genre(""), year(-1){
  connect(ripSelectedTracks, SIGNAL(clicked()), this, SLOT(startSession()));
  connect(editTag, SIGNAL(clicked()), this, SLOT(editInformation()));
  connect(trackListing, SIGNAL(clicked( QListViewItem * )), this, SLOT(selectTrack(QListViewItem*))); 
  connect(trackListing, SIGNAL(doubleClicked(QListViewItem *)), this, SLOT(editInformation()));
  connect(refreshList, SIGNAL(clicked()), this, SIGNAL(refreshCd()));
  connect(selectAllTracksButton, SIGNAL(clicked()), this, SLOT(selectAllTracks()));
  connect(deselectAllTracksButton, SIGNAL(clicked()), this, SLOT(deselectAllTracks()));
  trackListing->setSorting(-1, false);
}

/**
 * Bring up the dialog to edit the information about this album.
 * If there is not currently selected track return.
 * If ok is pressed then store the information and update track name.
 */
void TracksConfigImp::editInformation(){
  QListViewItem * currentItem = trackListing->currentItem();
  if( currentItem == 0 ){
    KMessageBox::sorry(this, i18n("Please select a track."), i18n("No Track Selected"));
    return;
  }

  // Create dialog.
  Id3TagDialog dialog(this, "Album info editor dialog", true);
  dialog.artist->setText(group);
  dialog.album->setText(album);
  dialog.year->setValue(year);
  dialog.trackLabel->setText(i18n("Track %1").arg(currentItem->text(HEADER_TRACK)));
  dialog.title->setText(currentItem->text(HEADER_NAME));
  int totalGenres = dialog.genre->count();
  if(genre == "")
    genre = i18n("Other");
  
  for(int i = 0; i < totalGenres; i++){
    if(dialog.genre->text(i) == genre){
      dialog.genre->setCurrentItem(i);
      break;
    }
  }

  // set focus to track title
  dialog.title->setFocus();
  dialog.title->selectAll();

  // Show dialog and save results.
  bool okClicked = dialog.exec();
  if(okClicked){
    group = dialog.artist->text();
    album = dialog.album->text();
    year = dialog.year->value();
    currentItem->setText(HEADER_NAME, dialog.title->text());
    genre = dialog.genre->currentText();

    QString newTitle = QString("%1 - %2").arg(group).arg(album);
    if(albumName->text() != newTitle)
      albumName->setText(newTitle);
  }
}

/**
 * Helper function.  Checks all tracks and then calls startSession to rip them all.
 */
void TracksConfigImp::ripWholeAlbum(){
  selectAllTracks();
  startSession();
}

/**
 * Start of the "ripping session" by emiting signals to rip the selected tracks.
 * If any album information is not set, notify the user first.
 */
void TracksConfigImp::startSession(){
  if(trackListing->childCount() == 0){
    KMessageBox:: sorry(this, i18n("No tracks are selected to rip. Please select at least 1 track before ripping."), i18n("No Tracks Selected"));
    return;
  }

  QString list = "";
  if( genre == "" )
    list += "Genre";
  
  if( year == 0 ){
    if(list != "")
      list += ", ";
    list += "Year";
  }
  if( group == "Unknown Artist"){
    if(list != "")
      list += ", ";
    list += "Artist";
  }
  if( album == "Unknown Album"){
    if(list != "")
      list += ", ";
    list += "Album";
  }
  if( list != ""){
    int r = KMessageBox::questionYesNo(this, i18n("Part of the album is not set: %1.\n (To change album information click the \"Edit Information\" button.)\n Would you like to rip the selected tracks anyway?").arg(list), i18n("Album Information Incomplete"));
    if( r == KMessageBox::No )
      return;
  }
  QListViewItem * currentItem = trackListing->firstChild();
  Job *lastJob = NULL;
  int counter = 0;
  while( currentItem != 0 ){
    if(currentItem->pixmap(HEADER_RIP) != NULL ){
      Job *newJob = new Job();
      newJob->album = album;
      newJob->genre = genre;
      newJob->group = group;
      newJob->song = currentItem->text(HEADER_NAME);
      newJob->track = currentItem->text(HEADER_TRACK).toInt();
      newJob->year = year;
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
void TracksConfigImp::selectTrack(QListViewItem *currentItem){
  if(!currentItem)
    return;
  if(currentItem->pixmap(HEADER_RIP) != NULL){
    QPixmap empty;
    currentItem->setPixmap(HEADER_RIP, empty);
  }
  else
    currentItem->setPixmap(HEADER_RIP, SmallIcon("check"));
}

/**
 * Turn on all of the tracks.
 */
void TracksConfigImp::selectAllTracks(){
  QListViewItem *currentItem = trackListing->firstChild();
  while( currentItem != 0 ){
    currentItem->setPixmap(HEADER_RIP, SmallIcon("check"));
    currentItem = currentItem->nextSibling();
  }
}

/**
 * Turn off all of the tracks.
 */
void TracksConfigImp::deselectAllTracks(){
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
void TracksConfigImp::newAlbum(QString newGroup, QString newAlbum, int newYear, QString newGenre){
  albumName->setText(QString("%1 - %2").arg(newGroup).arg(newAlbum));
  trackListing->clear();
  album = newAlbum;
  group = newGroup;
  year = newYear;
  genre = newGenre;
}

/**
 * There is a new song for this album.  Add it to the list of songs.  Set the current selected
 * song to the first one.
 * @param track the track number for the song.
 * @param song the name of the song.
 */
void TracksConfigImp::newSong(int track, QString song, int length){
  song = song.mid(song.find(' ',0)+1, song.length());
  song = KURL::decode_string(song);
  song.replace(QRegExp("/"), "-");
  QString songLength = QString("%1:%2%3").arg(length/60).arg((length % 60)/10).arg((length % 60)%10);
  QListViewItem * newItem = new QListViewItem(trackListing, "", QString("%1").arg(track), songLength, song);
  trackListing->setCurrentItem(trackListing->firstChild());
}

#include "tracksconfigimp.moc"

// trackconfigimp.cpp

