/**
 * Copyright (c) 1997 Bernd Johannes wuebben@math.cornell.edu
 * Copyright (c) 2002-2003 Aaron J. Seigo <aseigo@kde.org>
 * Copyright (c) 2004 Alexander Kern <alex.kern@gmx.de>
 * Copyright (c) 2003-2006 Richard Lärkäng <nouseforaname@home.se>
 * Copyright (c) 2010 Gerd Fleischer gerdfleischer at web dot de
 *
 * --------------
 * ISI KsCD Team :
 * --------------
 * Bouchikhi Mohamed-Amine <bouchikhi.amine@gmail.com>
 * Gastellu Sylvain
 * -----------------------------------------------------------------------------
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef AUDIOCD_H
#define AUDIOCD_H

#include <solid/opticaldisc.h>
#include <solid/device.h>
#include <solid/opticaldrive.h>
#include <solid/block.h>
#include <solid/devicenotifier.h>

#include <phonon/mediasource.h>
#include <phonon/mediaobject.h>
#include <phonon/mediacontroller.h>

#include <QString>
#include <QObject>

class AudioCD: public QObject
{
    Q_OBJECT

public:
    enum DriveStatus {
        NoDisc,
        Loading,
        Ready,
        ReadyNoAudio
    };

private:
    Solid::DeviceNotifier *bell;
    Solid::Device odsign;
    Solid::OpticalDrive *cdDrive;
    Solid::OpticalDisc *cd;
    Solid::Block *block;
    Phonon::MediaSource *src;
    Phonon::MediaObject *obj;
    Phonon::MediaController *ctlr;
    DriveStatus status;
    QString discUdi;
    uint tracks;
    int discLength;
    QList<uint> offsetList, trackLengthList;
    QString freeDbId, musicbrainzId;
    void getDiscParameter();

private slots:
    void discInfoChanged();
    void registerMediaSource();
    void discEjected(const QString &);

public:
    AudioCD();
    AudioCD(Solid::Device aCd);
    ~AudioCD();
    Solid::OpticalDrive * getCdDrive() const;
    Solid::OpticalDisc * getCd() const;
    Phonon::MediaSource * getMediaSource() const;
    QString getCdPath() const;
    bool isCdInserted() const;
    bool hasAudio() const;
    DriveStatus getDriveStatus() const;
    QString getDriveUdi() const;
    QString getDiscUdi() const;
    uint getTrackNum() const;
    QList<uint> getOffsetList() const;
    int getDiscLength() const;
    uint getTrackLength(int) const;
    QString getFreeDbId() const;
    QString getMusicbrainzId() const;
    QStringList metaData();
    Phonon::State state();
        
public slots:
    void reloadCD();
    void eject();

signals:
    void driveStatusChanged(AudioCD::DriveStatus);
    void newDiscInfo();
};

#endif
