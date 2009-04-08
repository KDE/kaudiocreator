/**
 * Copyright (C) 2003-2005 Benjamin C Meyer (ben at meyerhome dot net)
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

#ifndef ENCODEFILEIMP_H
#define ENCODEFILEIMP_H

#include "config-kaudiocreator.h"

#include "libkcddb/genres.h"
#include "ui_encodefile.h"
#include "qmap.h"
#include <kpagedialog.h>

#define COLUMN_FILE 0
#define COLUMN_TITLE 1
#define COLUMN_ARTIST 2
#define COLUMN_ALBUM 3
#define COLUMN_COMMENT 4
#define COLUMN_GENRE 5
#define COLUMN_TRACK 6
#define COLUMN_YEAR 7
#define COLUMN_ENCODER 8

class Job;

/**
 * This class lets the user encode a file.
 */
class EncodeFileImp : public KDialog, public Ui::EncodeFile  {

Q_OBJECT

signals:
  // Start encoding this wav file
  void startJob(Job *newJob);

public:
  EncodeFileImp(QWidget* parent = 0);

protected slots:
	void openFiles();
	void openDirectory();
	void clearFileList();
	void removeSelectedFiles();

	void assignArtist();
	void assignAlbum();
	void assignComment();
	void assignGenre();
	void assignTrack();
	void assignYear();
	void assignEncoder();
	void assignAll();

	void setupEncoderBox();
	void editFile(QTreeWidgetItem *, int);
	void closeEditor();
	// Encode button
	void encode();
	void encodeAndClose();

private:
	// List of genres and i18n versions
	KCDDB::Genres m_genres;
	QTreeWidgetItem *editedItem;
	int editedColumn;
	QString fileTypeFilter;
	QStringList dirFilter;
	QMap<QString, QStringList> encoderMap;
#ifdef HAVE_TAGLIB
	QStringList taglibExtensions;
#endif

	void setupGlobals();
	void addFilesToList(const QStringList &);
};

#endif // ENCODEFILEIMP_H

