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

#include "audiocd.h"

#include <solid/device.h>
#include <solid/devicenotifier.h>
#include <klocale.h>
#include <KCddb/Client>
#include <kdebug.h>

#include "ui_tracks.h"

class Job;
class QStandardItemModel;
class QStandardItem;

/**
 * This class handles the display of the tracks. It also starts off the job que.
 */
class TracksImp : public QWidget, public Ui::Tracks
{

Q_OBJECT

signals:
	void ripTrack(Job *job);
	void driveStatusChanged(AudioCD::DriveStatus);
	void hasTracks(bool);
/*	void renameTrack(QTreeWidgetItem *);*/
	void sessionStarted();

public:
	TracksImp( QWidget* parent = 0);
	~TracksImp();

	bool hasCD();
    bool hasAudio() const;
	void setDevice(const QString &);

public slots:
	void loadSettings();
	void initDevice();

	// Toolbar Buttons
	void startSession( QString encoder = QString() );
	void editInformation();
//     void editCurrentTrack();
	void performCDDB();
	void ejectDevice(const QString &deviceToEject);
	void eject();
	void selectAllTracks();
	void deselectAllTracks();

private slots:
	void newDisc();
    void discChanged(AudioCD::DriveStatus);
	void changeDevice(const QString &);
    void registerDevice(const QString &udi);
    void unregisterDevice(const QString &udi);
    void lookupCDDBDone(KCDDB::Result result);
	void artistChangedByUser();
	void albumChangedByUser();
    void commentChangedByUser();
    void assignArtisToTracks();
    void assignCommentToTracks();
	void yearChangedByUser(int);
	void genreChangedByUser(const QString &);
	void syncToCddbInfo(QStandardItem *);

private:
	void lookupDevice();
	void lookupCDDB();
	void newAlbum();
	void setAlbumInfo(const QString &, const QString &);
	void ripWholeAlbum();
	QList<int> selectedTracks();
	void toggleInputs(bool);
	QString formatTime(unsigned s);

	KCDDB::Client* cddb;
    QStandardItemModel *trackModel;
//	KCompactDisc* cd;
    QHash<QString, Solid::Device> devMap;
    QHash<QString, QString> udiMap;
    AudioCD *currentDrive;
    Solid::DeviceNotifier *bell;

	// Current album
	KCDDB::CDInfo cddbInfo;
};

#endif // TRACKSIMP_H
