/***************************************************************************
                                cdconfig.h
                             -------------------
    begin                : Friday Feb 9 2002
    copyright            : (C) 2001 by Benjamin Meyer
    email                : ben-dev@meyerhome.net
 ***************************************************************************/

#ifndef CDCONFIGIMP_H
#define CDCONFIGIMP_H

#include <qwidget.h>
#include "cdconfig.h"
#include <kjanuswidget.h>
#include <kurl.h>
#include <qtimer.h>

class Job;

class CdConfigImp : public CdConfig  {

Q_OBJECT

signals:
  void newAlbum(QString group, QString album, int year, QString genre);
  void newSong(int track, QString song, int length);
  void ripAlbum();

public:
  CdConfigImp( QWidget* parent = 0, const char* name = 0);
  ~CdConfigImp();

public slots:
  void timerDone();

private slots:
  void attemptToListAlbum();

private:
  QTimer *timer;

  protected:

    enum FileType
    {
      FileTypeUnknown,
      FileTypeOggVorbis,
      FileTypeMP3,
      FileTypeWAV
    };

    enum DirType
    {
      DirTypeUnknown,
      DirTypeDevice,
      DirTypeByName,
      DirTypeByTrack,
      DirTypeTitle,
      DirTypeInfo,
      DirTypeRoot,
      DirTypeMP3
    };

    void                  writeHeader(long);
    struct cdrom_drive *  findDrive(bool &noPermission);
    void                  parseArgs(const KURL &);

    void getParameters();

    struct cdrom_drive *  initRequest(const KURL &);
    uint                  discid(struct cdrom_drive *);
    int                  updateCD(struct cdrom_drive *);

    FileType fileType(const QString & filename);

    class Private;
    Private * d;
    
  private:
    cdrom_drive * pickDrive();
    unsigned int get_discid(cdrom_drive *);
    bool updating;

};

#endif

// cdconfig.h

