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

