/***************************************************************************
                                job.h
                             -------------------
    begin                : Friday Feb 9 2002
    copyright            : (C) 2001 by Benjamin Meyer
    email                : ben-dev@meyerhome.net
 ***************************************************************************/

#ifndef JOB_H
#define JOB_H

/**
 * The job class is what is passed around the system.  All of the data about
 * the file beeing ripped and encoded is stored in here.
 */
class Job{

public:
  inline Job():id(-1),genre("Other"),group(""),album(""),song(""), track(-1),year(-1),lastSongInAlbum(false){};
  int id; 
  QString genre;		
  QString group;
  QString album;
  QString song;
  int track;
  int year;

  QString location;
  QString newLocation;
  QString errorString;
  bool lastSongInAlbum;
}; 

#endif

// job.h

