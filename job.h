/**
 * Copyright (C) 2003-2005 Benjamin C Meyer (ben at meyerhome dot net)
 * Copyright (C) 2009 Gerd Fleischer (gerdfleischer at web dot de)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef JOB_H
#define JOB_H

#include <klocale.h>
#include <QHash>

/**
 * The job class is what is passed around the system.  All of the data about
 * the file being ripped and encoded is stored in here.
 */
class Job
{

public:
	inline Job():id(-1),track_title(""),track_artist(""), track(-1),track_comment(""), year(-1), genre(i18n("Other")), group(""), album(""), comment(""), lastSongInAlbum(false), removeTempFile(true), encoder(-1) {}

	QString replaceSpecialChars(const QString &string, bool quote, QHash<QString,QString> map, bool createFilename=false);

	void fix( const QString &in, const QString &out );
	const QString make_fat32_compatible(const QString &tag);
	
	// The device to obtain the file such as /dev/cdrom/ (Used when ripping and ejecting)
	QString device;
	int id; 

	QString track_title;
	QString track_artist;
	int track;
	QString track_comment;
	int year;
	QString genre;

	QString group;
	QString album;
	QString comment;

	// Currently location of file X
	QString location;

	// New location of a file after current opertation is complete (rip/encode).
	QString newLocation;

	// What was just attempted to do via this job and is spit out in the event
	// of an error.
	QString errorString;

	// If this is the last track to be ripped then value is true. 
	bool lastSongInAlbum;

	// If the file should be removed on errorr or when finished ripping
	bool removeTempFile;
	bool checkOutput;

	// output from the processing.
	QString output;

	QString encoder;
}; 

#endif // JOB_H

