/*
   Copyright (C) 2000 Michael Matz <matz@kde.org>

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
   Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/
#ifndef _CDDB_H_
#define _CDDB_H_

#include <qcstring.h>
#include <qvaluelist.h>
#include <qstringlist.h>

class QFile;
class QTextStream;
class KExtendedSocket;

class CDDB {
public:
  CDDB();
  ~CDDB();
  bool set_server(const char *hostname = 0, unsigned short int port = 0);
  void add_cddb_dirs(const QStringList& list);
  void save_cddb (bool save) { save_local = save; }
  unsigned int get_discid(QValueList<int>& track_ofs);
  bool queryCD(QValueList<int>& track_ofs);
  QString title() const { return m_title; }
  QString artist() const { return m_artist; }
  QString category() const { return m_category; }
  int trackCount() const { return m_tracks; }
  QString track(int i) const;
private:
  bool readLine(QCString& s);
  bool writeLine(const QCString& s);
  bool deinit();
  bool parse_read_resp(QTextStream*, QTextStream*);
  bool searchLocal(unsigned int id, QFile *ret_file);
  KExtendedSocket *ks;
  QCString h_name;
  unsigned short int port;
  bool remote;
  bool save_local;
  QStringList cddb_dirs;
  QCString buf;
  unsigned int m_discid;

  int m_tracks;
  QString m_title;
  QString m_artist;
  QString m_category;
  QStringList m_names;
};

#endif
