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

// Clean up the string so that it doesn't wander off to unexpected directories
static QString sanitize(const QString &s)
{
  QString result = s;
  result.replace('/', ":");
  if (result.isEmpty())
    result = "(empty)";
  if (result[0] == '.')
    result[0] = '_';
  return result;
}

/**
 * A helper function to replace %X with the stuff in the album.
 * if quote is true then put "" around the %X
 */
void Job::replaceSpecialChars(QString &str, bool quote, QMap<QString, QString> _map){
  QMap<QString,QString> map = _map;
  
  map.insert("album", sanitize(album));
  map.insert("genre", sanitize(genre));
  map.insert("artist", sanitize(group));
  map.insert("year", QString::number(year));
  map.insert("song", sanitize(song));
  if( track < 10 )
      map.insert("track", "0" + QString::number(track) );
  else
      map.insert("track", QString::number(track) );

  if (quote)
      str = KMacroExpander::expandMacrosShellQuote(str, map);
  else
      str = KMacroExpander::expandMacros(str, map);
}
