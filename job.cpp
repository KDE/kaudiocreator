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
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 **/

#include "job.h"
#include <kmacroexpander.h>
#include <tqregexp.h>

/**
 * A helper function to replace %X with the stuff in the album.
 * if quote is true then put "" around the %X
 */
TQString Job::replaceSpecialChars(const TQString &string, bool quote, TQMap<TQString, TQString> _map){
	TQMap<TQString,TQString> map = _map;

  map.insert("title", track_title);
  map.insert("artist", track_artist);
  map.insert("number", TQString().sprintf("%02d", track));
  map.insert("comment", track_comment);
  map.insert("year", TQString::number(year));
	map.insert("genre", genre);

  map.insert("albumtitle", album);
  map.insert("albumcomment", comment);
  map.insert("albumartist", group);

	if (quote)
      return (KMacroExpander::expandMacrosShellQuote(string, map));
  else
      return (KMacroExpander::expandMacros(string, map));
}

void Job::fix(const TQString &in, const TQString &out){
	track_title.replace( TQRegExp(in), out );
	track_artist.replace( TQRegExp(in), out );
	track_comment.replace( TQRegExp(in), out );
	// year
	// track
	genre.replace( TQRegExp(in), out );
	album.replace( TQRegExp(in), out );
	comment.replace( TQRegExp(in), out );
	group.replace( TQRegExp(in), out );
}

