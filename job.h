/**
 * This file is part of the KAudioCreator package
 * Copyright (C) 2003 Benjamin C Meyer (ben+kaudiocreator at meyerhome dot net)
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

#include <tqmap.h>
#include <klocale.h>

/**
 * The job class is what is passed around the system.  All of the data about
 * the file being ripped and encoded is stored in here.
 */
class Job{

public:
	inline Job():id(-1),track_title(""),track_artist(""), track(-1),track_comment(""), year(-1), genre(i18n("Other")), group(""), album(""), comment(""), lastSongInAlbum(false), removeTempFile(true), encoder(-1) {};

	TQString replaceSpecialChars(const TQString &string, bool quote, TQMap<TQString,TQString> map);

	void fix( const TQString &in, const TQString &out );
	
	// The device to obtain the file such as /dev/cdrom/ (Used when ripping and ejecting)
	TQString device;
	int id; 

	TQString track_title;
	TQString track_artist;
	int track;
	TQString track_comment;
	int year;
	TQString genre;

	TQString group;
	TQString album;
	TQString comment;

	// Currently location of file X
	TQString location;

	// New location of a file after current opertation is complete (rip/encode).
	TQString newLocation;

	// What was just attempted to do via this job and is spit out in the event
	// of an error.
	TQString errorString;

	// If this is the last track to be ripped then value is true. 
	bool lastSongInAlbum;

	// If the file should be removed when finished encoding
	bool removeTempFile;

	// output from the processing.
	TQString output;

	int encoder;
}; 

#endif // JOB_H

