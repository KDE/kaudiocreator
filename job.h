#ifndef JOB_H
#define JOB_H

class Job{

public:
  inline Job():id(-1),genre("other"),group(""),album(""),song(""), track(-1),year(-1),lastSongInAlbum(false){};
  int id; 
  QString genre;		
  QString group;
  QString album;
  QString song;
  int track;
  int year;
  int length;

  QString location;
  QString newLocation;
  QString errorString;
  bool lastSongInAlbum;
}; 

#endif

// job.h

