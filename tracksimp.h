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

// CDDB support via libkcddb
#include <libkcddb/client.h>

using namespace KCDDB;

class QListViewItem;
class Job;
class AlbumEditor;
class KProcess;

namespace KCDDB
{
  class CDInfo;
} ;

/**
 * This class handles the display of the tracks. It also starts off the job que.
 */
class TracksImp : public Tracks  {

Q_OBJECT

signals:
  void ripTrack(Job *job);
  void hasCD(bool);
  void hasTracks(bool);
 
public:
  TracksImp( QWidget* parent = 0, const char* name = 0);
  ~TracksImp();

  // This is public so the file encoder can use it
  QMap<QString, QString> genres;

public slots:
  void loadSettings();
  
  // Toolbar Buttons
  void startSession();
  void editInformation();
  void performCDDB();
  void ejectDevice(const QString &deviceToEject);
  void eject();
  void selectAllTracks();
  void deselectAllTracks();

private slots:
  void ejectDone(KProcess *proc);
  void selectTrack(QListViewItem *);
  void keyPressEvent(QKeyEvent *event);
  void editNextTrack();
  void editPreviousTrack();
 
  void timerDone();
  void changeDevice(const QString &file);
  void cddbDone(CDDB::Result result);

private:

  void cddbCD();
  void newAlbum(const QString &group = i18n("Unknown Artist"),
		const QString &album = i18n("Unknown Album"),
		uint year = 0, const QString &genre = "",
		uint revision = 0, const QString &category = "",
		const QString& comment = "");
  void newSong(int track, const QString &title, int length, const QString& comment); 
  void ripWholeAlbum();

  void editOtherTrack(bool nextOneUp);
  void setCdInfo(KCDDB::CDInfo &info);
  
  AlbumEditor *dialog;
  KCDDB::Client* cddb;
  
  unsigned long CDid;
  int dstatus;
  
  // Current album
  QString album;
  QString group;
  QString genre;
  QString comment;
  int year;
  // CDDB data, which we keep loaded so it doesn't
  // get lost when saving new cddb-information
  int revision;
  QString category;
  
  // Settings
  QString device;
};

#endif

