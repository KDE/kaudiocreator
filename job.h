/***************************************************************************
                                job.h
                             -------------------
    begin                : Friday Feb 9 2002
    copyright            : (C) 2001 by Benjamin Meyer
    email                : ben-dev@meyerhome.net
 ***************************************************************************/

#ifndef JOB_H
#define JOB_H

#include <qmap.h>

#include <klocale.h>

/**
 * The job class is what is passed around the system.  All of the data about
 * the file beeing ripped and encoded is stored in here.
 */
class Job{

public:
  inline Job():id(-1),genre(i18n("Other")),group(""),album(""),song(""), track(-1),year(-1),lastSongInAlbum(false){};

  void replaceSpecialChars(QString &string, bool quote, QMap<QString,QString> map);

  int id; 
  QString genre;		
  QString group;
  QString album;
  QString song;
  int track;
  int year;

  // Currently location of file X
  QString location;

  // New location of a file after current opertation is complete (rip/encode).
  QString newLocation;

  // What was just attempted to do via this job and is spit out in the event of an error.
  QString errorString;

  // If this is the last song in to be ripped then value is true. 
  bool lastSongInAlbum;
}; 

#endif

