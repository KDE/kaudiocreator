#include "tracksconfigimp.h"
#include <qlabel.h>
#include <qlistview.h>
#include <qpushbutton.h>
#include <qheader.h> 
#include <qpixmap.h>
#include <kiconloader.h>
#include "job.h"
#include "id3tagdialog.h"
#include <qlineedit.h>
#include <qspinbox.h>
#include <qcombobox.h>
#include <kmessagebox.h>
#include <klocale.h>

#define HEADER_TRACK 0
#define HEADER_NAME 1
#define HEADER_LENGTH 2
#define HEADER_RIP 3

/**
 * Constructor, connect up slots and signals.
 */
TracksConfigImp::TracksConfigImp( QWidget* parent, const char* name):TracksConfig(parent,name), album(""), group(""), genre(""), year(-1),allOn(false){
  connect(ripSelectedTracks, SIGNAL(clicked()), this, SLOT(startSession()));
  connect(editTag, SIGNAL(clicked()), this, SLOT(editInformation()));
  connect(trackListing, SIGNAL(clicked( QListViewItem * )), this, SLOT(selectTrack(QListViewItem*))); 
  connect(trackListing->header(), SIGNAL(clicked(int)), this, SLOT(headerClicked(int)));
  connect(refreshList, SIGNAL(clicked()), this, SIGNAL(refreshCd()));
  trackListing->setSorting(-1, false);
}

/**
 * Bring up the dialog to edit the information about this album.
 */
void TracksConfigImp::editInformation(){
  QListViewItem * currentItem = trackListing->firstChild();
  int counter = 0;
  while( currentItem != 0 ){
    if(currentItem->isSelected())
      break;
    currentItem = currentItem->nextSibling();
    counter++;
  }
  if( currentItem == 0 ){
    KMessageBox::sorry(this, i18n("Please select a track"), i18n("No track selected"));
    return;
  }

  Id3TagDialog dialog(this, "info dialog", true);
  dialog.artist->setText(group);
  dialog.album->setText(album);
  dialog.year->setValue(year);
  dialog.trackLabel->setText(QString("Track %1").arg(counter+1));
  dialog.title->setText(currentItem->text(1));
  int totalGenres = dialog.genre->count();
  if(genre == "")
    genre = "Other";
  for(int i = 0; i < totalGenres; i++){
    if(dialog.genre->text(i) == genre){
      dialog.genre->setCurrentItem(i);
      break;
    }
  }
  
  bool okClicked = dialog.exec();
  if(okClicked){
    group = dialog.artist->text();
    album = dialog.album->text();
    year = dialog.year->value();
    currentItem->setText(1, dialog.title->text());
    genre = dialog.genre->currentText();
  }
  
}

/**
 * Helper function.  Selects all tracks and then rips.
 */
void TracksConfigImp::ripWholeAlbum(){
  allOn = false;
  headerClicked(3);
  startSession();
}

/**
 * Start of the ripping session by emiting signals to rip the selected tracks.
 */
void TracksConfigImp::startSession(){
  QString list = "";
  if( genre == "" )
    list += "Genre";
  
  if( year == 0 ){
    if(list != "")
      list += ", ";
    list += "Year";
  }
  if( group == "No Artist"){
    if(list != "")
      list += ", ";
    list += "Artist";
  }
  if( album == "No Album"){
    if(list != "")
      list += ", ";
    list += "Album";
  }
  if( list != ""){
    int r = KMessageBox:: questionYesNo(this, QString("Part of the album is not set: %1. Would you like to continue anyway?").arg(list), "Album Information incomplete");
    if( r == KMessageBox::No )
      return;
  }
  QListViewItem * currentItem = trackListing->firstChild();
  int counter = 1;
  Job *last = NULL;
  while( currentItem != 0 ){
    if(currentItem->pixmap(HEADER_RIP) != NULL ){
      Job *j = new Job();
      j->album = album;
      j->genre = genre;
      j->group = group;
      j->song = currentItem->text(HEADER_NAME);
      j->track = counter;
      j->year = year;
      last = j;
      emit( ripTrack(j) ); 
    }
    counter++;
    currentItem = currentItem->nextSibling();
  }
  if(last)
    last->lastSongInAlbum = true;
}

/**
 * The header was clicked so turn them all on/off
 */
void TracksConfigImp::headerClicked(int){
  allOn = !allOn;
  QListViewItem * currentItem = trackListing->firstChild();
  while( currentItem != 0 ){
    if(!allOn){
      QPixmap a;
      currentItem->setPixmap(HEADER_RIP,a);
    }
    else{
      currentItem->setPixmap(HEADER_RIP, SmallIcon("check"));
    }
    currentItem = currentItem->nextSibling();
  }
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
 * Set the current stats for the album being displayed.
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
 * There is a new song for this album.  Add it to the list of songs.
 * @param track the track number for the song.
 * @param song the name of the song.
 */
void TracksConfigImp::newSong(int track, QString song, int length){
  song = song.mid(song.find(' ',0)+1, song.length());
  QString songLength = QString("%1:%2%3").arg(length/60).arg((length % 60)/10).arg((length % 60)%10);
  QListViewItem * newItem = new QListViewItem(trackListing, QString("%1").arg(track), song, songLength, "");
  trackListing->setCurrentItem(0);
}

#include "tracksconfigimp.moc"

// trackconfigimp.cpp

