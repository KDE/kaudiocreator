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

#ifndef JOB_H
#define JOB_H

#include <qmap.h>
#include <klocale.h>

/**
 * The job class is what is passed around the system.  All of the data about
 * the file being ripped and encoded is stored in here.
 */
class Job{

public:
  inline Job():id(-1),genre(i18n("Other")),group(""),album(""),comment(""), track(-1),year(-1),song(""), song_comment(""), song_artist(""), lastSongInAlbum(false){};

  void replaceSpecialChars(QString &string, bool quote, QMap<QString,QString> map);

  // The device to obtain the file such as /dev/cdrom/ (Used when ripping and ejecting)
  QString device;
  int id; 

  QString genre;		
  QString group;
  QString album;
  QString comment;
  int track;
  int year;
  
  /**
   * TODO
   * Change to:
   * track_title
   * track_comment
   * track_artist;
   */
  QString song;
  QString song_comment;
  QString song_artist;
 
  // Currently location of file X
  QString location;

  // New location of a file after current opertation is complete (rip/encode).
  QString newLocation;

  // What was just attempted to do via this job and is spit out in the event
  // of an error.
  QString errorString;

  // If this is the last song in to be ripped then value is true. 
  bool lastSongInAlbum;

  // output from the processing.
  QString output;
}; 

#endif

