/*
 *  Copyright (c) 2003 Waldo Bastian <bastian@kde.org>
 *  Copyright (C) 2003-2005 Benjamin C. Meyer <ben at meyerhome dot net>
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

#include "prefs.h"
#include "job.h"

#include <kmacroexpander.h>

#include <QRegExp>

/**
 * A helper function to replace %X with the stuff in the album.
 * if quote is true then put "" around the %X
 */
QString Job::replaceSpecialChars(const QString &string, bool quote, QHash<QString, QString> _map, bool createFilename)
{
	QHash<QString,QString> map = _map;
  
	map.insert("title", track_title);
	map.insert("artist", track_artist);
	map.insert("number", track == EMPTY_TRACK ? QString() : QString().sprintf("%02d", track));
	map.insert("comment", track_comment);
	map.insert("year", year == EMPTY_YEAR ? QString() : QString::number(year));
	map.insert("genre", genre);
	
	map.insert("albumtitle", album);
	map.insert("albumcomment", comment);
	map.insert("albumartist", group);
	map.insert("albumartistinitial", !group.isEmpty() ? group.at(0).toUpper() : QString() );

	if (createFilename && Prefs::fat32compatible()) {
		map.insert("title", make_fat32_compatible(map["title"]));
		map.insert("artist", make_fat32_compatible(map["artist"]));
		map.insert("number", make_fat32_compatible(map["number"]));
		map.insert("comment", make_fat32_compatible(map["comment"]));
		map.insert("year", make_fat32_compatible(map["year"]));
		map.insert("genre", make_fat32_compatible(map["genre"]));
		
		map.insert("albumtitle", make_fat32_compatible(map["albumtitle"]));
		map.insert("albumcomment", make_fat32_compatible(map["albumcomment"]));
		map.insert("albumartist", make_fat32_compatible(map["albumartist"]));
		map.insert("albumartistinitial", make_fat32_compatible(map["albumartistinitial"]));
	}

	if (quote)
		return (KMacroExpander::expandMacrosShellQuote(string, map));
	else
		return (KMacroExpander::expandMacros(string, map));
}

void Job::fix(const QString &in, const QString &out)
{
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

//remove \ / : * ? " < > |
const QString Job::make_fat32_compatible(const QString &tag)
{
	QString s = tag;
	QString rep = Prefs::replaceFatIncompatible();
	s.replace("\\", rep);
	s.replace("/", rep);
	s.replace(":", rep);
	s.replace("*", rep);
	s.replace("?", rep);
	s.replace("<", rep);
	s.replace(">", rep);
	s.replace("|", rep);
	return s;
}
