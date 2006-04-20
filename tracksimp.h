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
 * the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#ifndef TRACKSIMP_H
#define TRACKSIMP_H

#include "tracks.h"
#include <kiconloader.h>
#include <klistview.h>
#include <klocale.h>

// CDDB support via libkcddb
#include <libkcddb/client.h>

#define HEADER_RIP 0
#define HEADER_TRACK 1
#define HEADER_LENGTH 2
#define HEADER_TRACK_NAME 3
#define HEADER_TRACK_ARTIST 4
#define HEADER_TRACK_COMMENT 5

using namespace KCDDB;
class Job;
class KProcess;
class KCompactDisc;

class TracksItem : public KListViewItem
{
public:
    TracksItem( KListView *parent, KListViewItem *after, QString t, QString a, int tr, QString l, QString c )
        : KListViewItem( parent, after, QString::null/*rip*/, QString::number(tr), l, t )
    {
        m_title = t;
        m_artist = a;
        m_length = l;
        m_track = tr;
        m_comment = c;
        m_checked = false;
    }

    QString title()     const { return m_title; }
    QString artist()    const { return m_artist; }
    int     track()     const { return m_track; }
    QString length()    const { return m_length; }
    bool    checked()   const { return m_checked; }
    QString comment()   const { return m_comment; }
    #include <kdebug.h>
    void    setTitle( const QString &t )  { m_title = t; kdDebug() << "title: " << m_title << endl; }
    void    setChecked( const bool &b )   { 
        m_checked = b;
        b ? setPixmap( HEADER_RIP, SmallIcon( "apply", height()-2 ) ) :
            setPixmap( HEADER_RIP, 0 );
    }

private:
    QString m_title;
    QString m_artist;
    int     m_track;
    QString m_length;
    QString m_comment;
    bool    m_checked; // marked for ripping
};



/**
 * This class handles the display of the tracks. It also starts off the job que.
 */
class TracksImp : public Tracks {

Q_OBJECT

signals:
	void ripTrack(Job *job);
	void hasCD(bool);
	void hasTracks(bool);
 
public:
	TracksImp( QWidget* parent = 0, const char* name = 0);
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

private slots:
    void changeDevice(const QString &file);
	void keyPressEvent(QKeyEvent *event);
	void lookupCDDBDone(CDDB::Result result);
    void newDisc(unsigned discId);   
    void selectTrack(QListViewItem *);   

private:
	void lookupDevice();
	void lookupCDDB();
	void newAlbum();
	void ripWholeAlbum();
    QPtrList<TracksItem> selectedTracks();   
    
	QString formatTime(unsigned ms);

	KCDDB::Client *cddb;
	KCompactDisc  *cd;

	// Current album
	KCDDB::CDInfo cddbInfo;
};

#endif // TRACKSIMP_H
