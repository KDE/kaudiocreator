/***************************************************************************
                              tracksimp.h
                             -------------------
    begin                : Friday Feb 9 2002
    copyright            : (C) 2001 by Benjamin Meyer
    email                : ben-dev@meyerhome.net
 ***************************************************************************/

#ifndef TRACKSIMP_H
#define TRACKSIMP_H

#include "tracks.h"
#include <klocale.h>

class QListViewItem;
class Job;
class Id3TagDialog;
class KProcess;

/**
 * This class handles the display of the tracks. It also starts off the job que.
 */
class TracksImp : public Tracks  {

Q_OBJECT

signals:
  void ripTrack(Job *job);
  
public:
  TracksImp( QWidget* parent = 0, const char* name = 0);
  ~TracksImp();

public slots:
  void loadSettings();
  
  // Toolbar Buttons
  void startSession();
  void editInformation();
  void performCDDB();
  void eject();

private slots:
  void ejectDone(KProcess *proc);
  void selectTrack(QListViewItem *);
  void selectAllTracks();
  void deselectAllTracks();
  void keyPressEvent(QKeyEvent *event);
  void editNextTrack();
  void editPreviousTrack();
 
  void timerDone();
  void changeDevice(const QString &file);

private:
  bool cddbCD();
  void newAlbum(QString group = i18n("Unknown Artist"),
		QString album = i18n("Unknown Album"),
		uint year = 0, QString genre = "");
  void newSong(int track, QString song, int length); 
  void ripWholeAlbum();
   
  Id3TagDialog *dialog;
  QMap<QString, QString> genres;
  unsigned long CDid;
  int dstatus;
  
  // Current album
  QString album;
  QString group;
  QString genre;
  int year;
  
  // Settings
  QString device;
  bool performCDDBauto;
  bool autoRip;
};

#endif

