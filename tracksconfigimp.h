/***************************************************************************
                              tracksconfigimp.h
                             -------------------
    begin                : Friday Feb 9 2002
    copyright            : (C) 2001 by Benjamin Meyer
    email                : ben-dev@meyerhome.net
 ***************************************************************************/

#ifndef TRACKSCONFIGIMP_H
#define TRACKSCONFIGIMP_H

#include "tracksconfig.h"

class QListViewItem;
class Job;

/**
 * This class handles the display of the tracks. It also starts off the job que.
 */
class TracksConfigImp : public TracksConfig  {

Q_OBJECT

signals:
  void updateProgress(int id, int progress);
  void ripTrack(Job *job);
  void refreshCd();
  
public:
  TracksConfigImp( QWidget* parent = 0, const char* name = 0);

public slots:
  void startSession();
  void newAlbum(QString group, QString album, int year, QString genre);
  void newSong(int track, QString song, int length);
  void ripWholeAlbum();
  void editInformation();

private slots:
  void selectTrack(QListViewItem *);
  void headerClicked(int);

private:
  QString album;
  QString group;
  QString genre;
  int year;
  bool allOn;
};

#endif

// tracksconfig.h

