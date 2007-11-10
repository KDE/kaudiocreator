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

#ifndef TRACKSIMP_H
#define TRACKSIMP_H


#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QKeyEvent>

#include <klocale.h>
#include <libkcddb/client.h>
#include <kdebug.h>

#include "ui_tracks.h"

#define HEADER_RIP 0
#define HEADER_TRACK 1
#define HEADER_LENGTH 2
#define HEADER_TRACK_NAME 3
#define HEADER_TRACK_ARTIST 4
#define HEADER_TRACK_COMMENT 5

class Job;
class KCompactDisc;

class TracksItem : public QTreeWidgetItem
{
public:
    TracksItem( QTreeWidget *parent, QString t, QString a, int tr, QString l, QString c )
        : QTreeWidgetItem( parent, QTreeWidgetItem::UserType )
    {
        m_title = t;
        m_artist = a;
        m_length = l;
        m_track = tr;
        m_comment = c;
    }

    QString title()     const { return m_title; }
    QString artist()    const { return m_artist; }
    int     track()     const { return m_track; }
    QString length()    const { return m_length; }
    QString comment()   const { return m_comment; }
    void    setTitle( const QString &t )  { m_title = t; kDebug() << "title: " << m_title; }

private:
    QString m_title;
    QString m_artist;
    int     m_track;
    QString m_length;
    QString m_comment;
};


/**
 * This class handles the display of the tracks. It also starts off the job que.
 */
class TracksImp : public QWidget, public Ui::Tracks
{

Q_OBJECT

signals:
	void ripTrack(Job *job);
	void hasCD(bool);
	void hasTracks(bool);
	void renameTrack(QTreeWidgetItem *);
 
public:
	TracksImp( QWidget* parent = 0);
	~TracksImp();

	bool hasCD();

public slots:
	void loadSettings();

	// Toolbar Buttons
	void startSession( int encoder = -1 );
	void editInformation();
	void performCDDB();
	void ejectDevice(const QString &deviceToEject);
	void eject();
	void selectAllTracks();
	void deselectAllTracks();
	void editTrackName(QTreeWidgetItem *);
	void closeEditor();

private slots:
	void newDisc(unsigned discId);

	void changeDevice(const QString &file);
	void lookupCDDBDone(KCDDB::Result result);

private:
	void lookupDevice();
	void lookupCDDB();
	void newAlbum();
	void ripWholeAlbum();
	QList<TracksItem *> selectedTracks();

	QString formatTime(unsigned s);

	KCDDB::Client* cddb;

	KCompactDisc* cd;

	QTreeWidgetItem *editedItem;

	// Current album
	KCDDB::CDInfo cddbInfo;
};

#endif // TRACKSIMP_H
