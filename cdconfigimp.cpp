/*
  Copyright (C) 2000 Rik Hemsley (rikkus) <rik@kde.org>
  Copyright (C) 2000, 2001 Michael Matz <matz@kde.org>
  Copyright (C) 2001 Carsten Duvenhorst <duvenhorst@m2.uni-hannover.de>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#include <qpushbutton.h>

#define HAVE_LAME 0

#include <config.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <kapplication.h>

#include <qfile.h>
#include <qstrlist.h>
#include <qdatetime.h>
#include <qregexp.h>

typedef Q_INT16 size16;
typedef Q_INT32 size32;

#include <kconfig.h>
#include <kglobal.h>
#include <qspinbox.h>
#include <qlineedit.h>
#include <qcheckbox.h>

extern "C"
{
#include <cdda_interface.h>
#include <cdda_paranoia.h>

/* This is in support for the Mega Hack, if cdparanoia ever is fixed, or we
   use another ripping library we can remove this.  */
#include <linux/cdrom.h>
#include <sys/ioctl.h>

#if HAVE_LAME
#include <lame/lame.h>
#endif

}
#include <kdebug.h>
#include <kurl.h>
#include <kprotocolmanager.h>
#include <kinstance.h>
#include <klocale.h>

#include "cdconfigimp.h"
#include "cddb.h"

using namespace KIO;

#define MAX_IPC_SIZE (1024*32)

#define DEFAULT_CD_DEVICE "/dev/cdrom"

#define DEFAULT_CDDB_SERVER "freedb.freedb.org:8880"

extern "C"
{
  int FixupTOC(cdrom_drive *d, int tracks);
}

int start_of_first_data_as_in_toc;
int hack_track;
/* Mega hack.  This function comes from libcdda_interface, and is called by
   it.  We need to override it, so we implement it ourself in the hope, that
   shared lib semantics make the calls in libcdda_interface to FixupTOC end
   up here, instead of it's own copy.  This usually works.
   You don't want to know the reason for this.  */
int FixupTOC(cdrom_drive *d, int tracks)
{
  int j;
  for (j = 0; j < tracks; j++) {
    if (d->disc_toc[j].dwStartSector < 0)
      d->disc_toc[j].dwStartSector = 0;
    if (j < tracks-1
        && d->disc_toc[j].dwStartSector > d->disc_toc[j+1].dwStartSector)
      d->disc_toc[j].dwStartSector = 0;
  }
  long last = d->disc_toc[0].dwStartSector;
  for (j = 1; j < tracks; j++) {
    if (d->disc_toc[j].dwStartSector < last)
      d->disc_toc[j].dwStartSector = last;
  }
  start_of_first_data_as_in_toc = -1;
  hack_track = -1;
  if (d->ioctl_fd != -1) {
    struct cdrom_multisession ms_str;
    ms_str.addr_format = CDROM_LBA;
    if (ioctl(d->ioctl_fd, CDROMMULTISESSION, &ms_str) == -1)
      return -1;
    if (ms_str.addr.lba > 100) {
      for (j = tracks-1; j >= 0; j--)
        if (j > 0 && !IS_AUDIO(d,j) && IS_AUDIO(d,j-1)) {
          if (d->disc_toc[j].dwStartSector > ms_str.addr.lba - 11400) {
            /* The next two code lines are the purpose of duplicating this
             * function, all others are an exact copy of paranoias FixupTOC().
             * The gory details: CD-Extra consist of N audio-tracks in the
             * first session and one data-track in the next session.  This
             * means, the first sector of the data track is not right behind
             * the last sector of the last audio track, so all length
             * calculation for that last audio track would be wrong.  For this
             * the start sector of the data track is adjusted (we don't need
             * the real start sector, as we don't rip that track anyway), so
             * that the last audio track end in the first session.  All well
             * and good so far.  BUT: The CDDB disc-id is based on the real
             * TOC entries so this adjustment would result in a wrong Disc-ID.
             * We can only solve this conflict, when we save the old
             * (toc-based) start sector of the data track.  Of course the
             * correct solution would be, to only adjust the _length_ of the
             * last audio track, not the start of the next track, but the
             * internal structures of cdparanoia are as they are, so the
             * length is only implicitely given.  Bloody sh*.  */
            start_of_first_data_as_in_toc = d->disc_toc[j].dwStartSector;
            hack_track = j + 1;
            d->disc_toc[j].dwStartSector = ms_str.addr.lba - 11400;
          }
          break;
        }
      return 1;
    }
  }
  return 0;
}

/* libcdda returns for cdda_disc_lastsector() the last sector of the last
   _audio_ track.  How broken.  For CDDB Disc-ID we need the real last sector
   to calculate the disc length.  */
long my_last_sector(cdrom_drive *drive)
{
  return cdda_track_lastsector(drive, drive->tracks);
}

enum Which_dir { Unknown = 0, Device, ByName, ByTrack, Title, Info, Root,
                 MP3, Vorbis };

class CdConfigImp::Private
{
  public:

    Private()
    {
      clear();
      discid = 0;
      cddb = 0;
      based_on_cddb = false;
      s_byname = i18n("By Name");
      s_bytrack = i18n("By Track");
      s_track = i18n("Track %1");
      s_info = i18n("Information");
      s_mp3  = "MP3";
      s_vorbis = "Ogg Vorbis";
    }

    void clear()
    {
      which_dir = Unknown;
      req_track = -1;
    }

    QString path;
    int paranoiaLevel;
    bool useCDDB;
    QString cddbServer;
    int cddbPort;
    unsigned int discid;
    int tracks;
    QString cd_title;
    QString cd_artist;
    QStringList titles;
    bool is_audio[100];
    CDDB *cddb;
    bool based_on_cddb;
    QString s_byname;
    QString s_bytrack;
    QString s_track;
    QString s_info;
    QString s_mp3;
    QString s_vorbis;

#if HAVE_LAME
  lame_global_flags *gf;
  int bitrate;
  bool write_id3;
#endif

    Which_dir which_dir;
    int req_track;
    QString fname;
};

CdConfigImp::CdConfigImp( QWidget* parent, const char* name):CdConfig(parent,name){
  d = new Private;
  d->cddb = new CDDB;
  //connect(getNow, SIGNAL(clicked()), this, SLOT(attemptToListAlbum()));
  KConfig &config = *KGlobal::config();
  config.setGroup("cdconfig");
  autoRip->setChecked(config.readBoolEntry("autoRip", false));
  databaseServer->setText(config.readEntry("databaseServer", "freedb.freedb.org"));
  databasePort->setValue(config.readNumEntry("databasePort", 8880));
  performCDDBauto->setChecked(config.readBoolEntry("performCDDBauto", false));
  bool constantlyScan = config.readBoolEntry("constantlyScan", false); 
 
  //attemptToListAlbu/m();
  updating = false;

  timer = new QTimer(this);
  connect(timer, SIGNAL(timeout()), this, SLOT(timerDone()));
  if(constantlyScan){
    timer->start(3000, false);
    config.writeEntry("constantlyScan", constantlyScan);
  }
}

CdConfigImp::~CdConfigImp()
{
  KConfig &config = *KGlobal::config();
  config.setGroup("cdconfig");
  config.writeEntry("databaseServer",databaseServer->text());
  config.writeEntry("databasePort",databasePort->value());
  config.writeEntry("performCDDBauto", performCDDBauto->isChecked());
  config.writeEntry("autoRip", autoRip->isChecked());

  delete d->cddb;
  delete d;
}

/**
 * check for a new cd and update
 */
void CdConfigImp::timerDone(){
  if(updating)
    return;
  updating = true;
  attemptToListAlbum();
  updating = false;
}

struct cdrom_drive *
CdConfigImp::initRequest(const KURL & url)
{

#if HAVE_LAME
  if (NULL == (d->gf = lame_init())) { // init the lame_global_flags structure with defaults
    //error(KIO::ERR_DOES_NOT_EXIST, url.path());
    return 0;
  }
  id3tag_init (d->gf);
#endif

	// first get the parameters from the Kontrol Center Module
  getParameters();

	// then these parameters can be overruled by args in the URL
  parseArgs(url);


  struct cdrom_drive *drive = pickDrive();

  if (0 == drive)
  {
    
    //error(KIO::ERR_DOES_NOT_EXIST, url.path());
    return 0;
  }

  if (0 != cdda_open(drive))
  {
    //error(KIO::ERR_CANNOT_OPEN_FOR_READING, url.path());
    return 0;
  }

  if(updateCD(drive) == -1)
    return 0;

  d->fname = url.filename(false);
  QString dname = url.directory(true, false);
  if (!dname.isEmpty() && dname[0] == '/')
    dname = dname.mid(1);

  /* A hack, for when konqi wants to list the directory audiocd:/Bla
     it really submits this URL, instead of audiocd:/Bla/ to us. We could
     send (in listDir) the UDS_NAME as "Bla/" for directories, but then
     konqi shows them as "Bla//" in the status line.  */
  if (dname.isEmpty() &&
      (d->fname == d->cd_title || d->fname == d->s_byname ||
       d->fname == d->s_bytrack || d->fname == d->s_info ||
       d->fname == d->s_mp3 || d->fname == d->s_vorbis || d->fname == "dev"))
    {
      dname = d->fname;
      d->fname = "";
    }

  if (dname.isEmpty())
    d->which_dir = Root;
  else if (dname == d->cd_title)
    d->which_dir = Title;
  else if (dname == d->s_byname)
    d->which_dir = ByName;
  else if (dname == d->s_bytrack)
    d->which_dir = ByTrack;
  else if (dname == d->s_info)
    d->which_dir = Info;
  else if (dname == d->s_mp3)
    d->which_dir = MP3;
  else if (dname == d->s_vorbis)
    d->which_dir = Vorbis;
  else if (dname.left(4) == "dev/")
    {
      d->which_dir = Device;
      dname = dname.mid(4);
    }
  else if (dname == "dev")
    {
      d->which_dir = Device;
      dname = "";
    }
  else
    d->which_dir = Unknown;

  d->req_track = -1;
  if (!d->fname.isEmpty())
    {
      QString n(d->fname);
      int pi = n.findRev('.');
      if (pi >= 0)
        n.truncate(pi);
      int i;
      for (i = 0; i < d->tracks; i++)
        if (d->titles[i] == n)
          break;
      if (i < d->tracks)
        d->req_track = i;
      else
        {
          /* Not found in title list.  Try hard to find a number in the
             string.  */
          unsigned int ui, j;
          ui = 0;
          while (ui < n.length())
            if (n[ui++].isDigit())
              break;
          for (j = ui; j < n.length(); j++)
            if (!n[j].isDigit())
              break;
          if (ui < n.length())
            {
              bool ok;
              /* The external representation counts from 1.  */
              d->req_track = n.mid(ui, j - i).toInt(&ok) - 1;
              if (!ok)
                d->req_track = -1;
            }
        }
    }
  if (d->req_track >= d->tracks)
    d->req_track = -1;

  kdDebug(7101) << "audiocd: dir=" << dname << " file=" << d->fname
    << " req_track=" << d->req_track << " which_dir=" << d->which_dir << endl;
  return drive;
}

  unsigned int
CdConfigImp::get_discid(struct cdrom_drive * drive)
{
  unsigned int id = 0;
  for (int i = 1; i <= drive->tracks; i++)
    {
      unsigned int n = cdda_track_firstsector (drive, i) + 150;
      if (i == hack_track)
        n = start_of_first_data_as_in_toc + 150;
      n /= 75;
      while (n > 0)
        {
          id += n % 10;
          n /= 10;
        }
    }
  unsigned int l = (my_last_sector(drive));
  l -= cdda_disc_firstsector(drive);
  l /= 75;
  id = ((id % 255) << 24) | (l << 8) | drive->tracks;
  return id;
}

int
CdConfigImp::updateCD(struct cdrom_drive * drive)
{
  unsigned int id = get_discid(drive);
  //BEN TODO
  //if (id == d->discid)
  //  return -1;
  
  d->discid = id;
  d->tracks = cdda_tracks(drive);
  d->cd_title = i18n("No Album");
  d->cd_artist = i18n("No Artist");
  d->titles.clear();
  QValueList<int> qvl;

  for (int i = 0; i < d->tracks; i++)
    {
      d->is_audio[i] = cdda_track_audiop(drive, i + 1);
      if (i+1 != hack_track)
        qvl.append(cdda_track_firstsector(drive, i + 1) + 150);
      else
        qvl.append(start_of_first_data_as_in_toc + 150);
    }
  qvl.append(cdda_disc_firstsector(drive));
  qvl.append(my_last_sector(drive));

  if (performCDDBauto->isChecked())
  {
    KApplication::setOverrideCursor(Qt::waitCursor);
    d->cddb->set_server(databaseServer->text().latin1(), databasePort->value());

    if (d->cddb->queryCD(qvl))
    {
      d->based_on_cddb = true;
      d->cd_title = d->cddb->title();
      d->cd_artist = d->cddb->artist();
      for (int i = 0; i < d->tracks; i++)
      {
        QString n;
        n.sprintf("%02d ", i + 1);
        d->titles.append (n + d->cddb->track(i));
      }
      KApplication::restoreOverrideCursor();
      return 0;
    }
    KApplication::restoreOverrideCursor();
  }

  d->based_on_cddb = false;
  for (int i = 0; i < d->tracks; i++)
    {
      QString num;
      int ti = i + 1;
      QString s;
      num.sprintf("%02d", ti);
      if (cdda_track_audiop(drive, ti))
        s = d->s_track.arg(num);
      else
        s.sprintf("data%02d", ti);
      d->titles.append( s );
    }
  return 0;
}

/**
 * Attempt to list the files.
 */ 
void CdConfigImp::attemptToListAlbum(){
  KURL url = "/";
  struct cdrom_drive *drive = pickDrive();

  if (0 == drive)
  {
    emit(newAlbum("No Artist","No Album", 0, "other"));
    //error(KIO::ERR_DOES_NOT_EXIST, url.path());
    return;
  }

  if (0 != cdda_open(drive))
  {
    emit(newAlbum("No Artist","No Album", 0, "other"));
    //error(KIO::ERR_CANNOT_OPEN_FOR_READING, url.path());
    return;
  }

  drive = initRequest(url);
  if (!drive){
    return;
  }

  emit(newAlbum(d->cd_artist,d->cd_title, 0, "other"));
  for (int i = d->tracks; i > 0; i--){
    if (d->is_audio[i-1])
    {
      long size = CD_FRAMESIZE_RAW *
        ( cdda_track_lastsector(drive, i) - cdda_track_firstsector(drive, i));
      long length_seconds = size / 176400;
      emit(newSong(i,(d->titles[i-1]),length_seconds));
    }
  }
  if(autoRip->isChecked())
    emit(ripAlbum());
  cdda_close(drive);
}

  struct cdrom_drive *
CdConfigImp::pickDrive()
{
  QCString path(QFile::encodeName(d->path));

  struct cdrom_drive * drive = 0;

  if (!path.isEmpty() && path != "/")
    drive = cdda_identify(path, CDDA_MESSAGE_PRINTIT, 0);

  else
  {
    drive = cdda_find_a_cdrom(CDDA_MESSAGE_PRINTIT, 0);

    if (0 == drive)
    {
      if (QFile(DEFAULT_CD_DEVICE).exists())
        drive = cdda_identify(DEFAULT_CD_DEVICE, CDDA_MESSAGE_PRINTIT, 0);
    }
  }

  if (0 == drive)
  {
    kdDebug(7101) << "Can't find an audio CD" << endl;
  }

  return drive;
}

  void
CdConfigImp::parseArgs(const KURL & url)
{
  QString old_cddb_server = d->cddbServer;
  int old_cddb_port = d->cddbPort;
  bool old_use_cddb = d->useCDDB;

  d->clear();

  QString query(KURL::decode_string(url.query()));

  if (query.isEmpty() || query[0] != '?')
    return;

  query = query.mid(1); // Strip leading '?'.

  QStringList tokens(QStringList::split('&', query));

  for (QStringList::ConstIterator it(tokens.begin()); it != tokens.end(); ++it)
  {
    QString token(*it);

    int equalsPos(token.find('='));

    if (-1 == equalsPos)
      continue;

    QString attribute(token.left(equalsPos));
    QString value(token.mid(equalsPos + 1));

    if (attribute == "device")
    {
      d->path = value;
    }

    else if (attribute == "paranoia_level")
    {
      d->paranoiaLevel = value.toInt();
    }
    else if (attribute == "use_cddb")
    {
      d->useCDDB = (0 != value.toInt());
    }
    else if (attribute == "cddb_server")
    {
      int portPos = value.find(':');

      if (-1 == portPos)
        d->cddbServer = value;

      else
      {
        d->cddbServer = value.left(portPos);
        d->cddbPort = value.mid(portPos + 1).toInt();
      }
    }
  }

  /* We need to recheck the CD, if the user either enabled CDDB now, or
     changed the server (port).  We simply reset the saved discid, which
     forces a reread of CDDB information.  */

  if ((old_use_cddb != d->useCDDB && d->useCDDB == true)
      || old_cddb_server != d->cddbServer
      || old_cddb_port != d->cddbPort)
    d->discid = 0;

  kdDebug(7101) << "CDDB: use_cddb = " << d->useCDDB << endl;

}

void CdConfigImp::getParameters() {

  KConfig *config;
  config = new KConfig("kcmaudiocdrc");

  config->setGroup("CDDA");

  if (!config->readBoolEntry("autosearch",true)) {
     d->path = config->readEntry("device",DEFAULT_CD_DEVICE);
   }

  d->paranoiaLevel = 1; // enable paranoia error correction, but allow skipping

  if (config->readBoolEntry("disable_paranoia",false)) {
    d->paranoiaLevel = 0; // disable all paranoia error correction
  }

  if (config->readBoolEntry("never_skip",true)) {
    d->paranoiaLevel = 2;  // never skip on errors of the medium, should be default for high quality
  }

  config->setGroup("CDDB");

  d->useCDDB = config->readBoolEntry("enable_cddb",true);

  QString cddbserver = config->readEntry("cddb_server",DEFAULT_CDDB_SERVER);
  int portPos = cddbserver.find(':');
  if (-1 == portPos) {
    d->cddbServer = cddbserver;
  } else {
    d->cddbServer = cddbserver.left(portPos);
    d->cddbPort = cddbserver.mid(portPos + 1).toInt();
  }

#if HAVE_LAME

  config->setGroup("MP3");

  int quality = config->readNumEntry("quality",2);

  if (quality < 0 ) quality = 0;
  if (quality > 9) quality = 9;

  int method = config->readNumEntry("encmethod",0);

  if (method == 0) { 
    
    // Constant Bitrate Encoding
    lame_set_VBR(d->gf, vbr_off);
    lame_set_brate(d->gf,config->readNumEntry("cbrbitrate",160));
    d->bitrate = lame_get_brate(d->gf);
    lame_set_quality(d->gf, quality);

  } else {
    
    // Variable Bitrate Encoding
    
    if (config->readBoolEntry("set_vbr_avr",true)) {

      lame_set_VBR(d->gf,vbr_abr);
      lame_set_VBR_mean_bitrate_kbps(d->gf, config->readNumEntry("vbr_average_bitrate",0));

      d->bitrate = lame_get_VBR_mean_bitrate_kbps(d->gf);

    } else {

      if (lame_get_VBR(d->gf) == vbr_off) lame_set_VBR(d->gf, vbr_default);

      if (config->readBoolEntry("set_vbr_min",true)) 
	lame_set_VBR_min_bitrate_kbps(d->gf, config->readNumEntry("vbr_min_bitrate",0));
      if (config->readBoolEntry("vbr_min_hard",true))
	lame_set_VBR_hard_min(d->gf, 1);
      if (config->readBoolEntry("set_vbr_max",true)) 
	lame_set_VBR_max_bitrate_kbps(d->gf, config->readNumEntry("vbr_max_bitrate",0));

      d->bitrate = 128;
      lame_set_VBR_q(d->gf, quality);
      
    }

    lame_set_bWriteVbrTag(d->gf, config->readBoolEntry("write_xing_tag",true));

  }

  switch (   config->readNumEntry("mode",0) ) {

    case 0: lame_set_mode(d->gf, STEREO);
                break;
    case 1: lame_set_mode(d->gf, JOINT_STEREO);
                break;
    case 2: lame_set_mode(d->gf,DUAL_CHANNEL);
                break;
    case 3: lame_set_mode(d->gf,MONO);
                break;
    default: lame_set_mode(d->gf,STEREO);
                break;
  }

  lame_set_copyright(d->gf, config->readBoolEntry("copyright",false));
  lame_set_original(d->gf, config->readBoolEntry("original",true));
  lame_set_strict_ISO(d->gf, config->readBoolEntry("iso",false));
  lame_set_error_protection(d->gf, config->readBoolEntry("crc",false));

  d->write_id3 = config->readBoolEntry("id3",true);

  if ( config->readBoolEntry("enable_lowpassfilter",false) ) {

    lame_set_lowpassfreq(d->gf, config->readNumEntry("lowpassfilter_freq",0));

    if (config->readBoolEntry("set_lowpassfilter_width",false)) {
      lame_set_lowpasswidth(d->gf, config->readNumEntry("lowpassfilter_width",0));
    }

  }

  if ( config->readBoolEntry("enable_highpassfilter",false) ) {

    lame_set_highpassfreq(d->gf, config->readNumEntry("highpassfilter_freq",0));

    if (config->readBoolEntry("set_highpassfilter_width",false)) {
      lame_set_highpasswidth(d->gf, config->readNumEntry("highpassfilter_width",0));
    }

  }
#endif // HAVE_LAME

  delete config;
  return;
}

// cdconfigimp.cpp

