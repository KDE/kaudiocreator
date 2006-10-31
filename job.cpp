/*
 *  Copyright (c) 2003 Waldo Bastian <bastian@kde.org>
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public
 *  License version 2 as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *  Boston, MA 02111-1307, USA.
 **/

#include "job.h"
#include <kmacroexpander.h>
#include <qregexp.h>

/**
 * A helper function to replace %X with the stuff in the album.
 * if quote is true then put "" around the %X
 */
QString Job::replaceSpecialChars(const QString &string, bool quote, QMap<QString, QString> _map){
	QMap<QString,QString> map = _map;

  map.insert("title", track_title);
  map.insert("artist", track_artist);
  map.insert("number", QString().sprintf("%02d", track));
  map.insert("comment", track_comment);
  map.insert("year", QString::number(year));
	map.insert("genre", genre);

  map.insert("albumtitle", album);
  map.insert("albumcomment", comment);
  map.insert("albumartist", group);

	if (quote)
      return (KMacroExpander::expandMacrosShellQuote(string, map));
  else
      return (KMacroExpander::expandMacros(string, map));
}

void Job::fix(const QString &in, const QString &out){
	track_title.replace( QRegExp(in), out );
	track_artist.replace( QRegExp(in), out );
	track_comment.replace( QRegExp(in), out );
	// year
	// track
	genre.replace( QRegExp(in), out );
	album.replace( QRegExp(in), out );
	comment.replace( QRegExp(in), out );
	group.replace( QRegExp(in), out );
}

