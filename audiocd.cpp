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

#include "audiocd.h"

#include <discid/discid.h>

#include <solid/opticaldisc.h>
#include <solid/device.h>
#include <solid/opticaldrive.h>
#include <solid/block.h>
#include <solid/devicenotifier.h>

#include <kdebug.h>

#include <phonon/mediasource.h>

#include <QString>
#include <QStringList>
#include <QTimer>

using namespace Phonon;

AudioCD::AudioCD()
{
    cdDrive = 0;
    cd = 0;
    block = 0;
    src = 0;
    ctlr = 0;
    status = NoDisc;
    driveUdi = QString();
    discUdi = QString();

    obj = new MediaObject(this);
    connect(obj, SIGNAL(metaDataChanged()), this, SLOT(discInfoChanged()));

    bell = Solid::DeviceNotifier::instance();
    connect(bell, SIGNAL(deviceAdded(const QString &)), this, SLOT(reloadCD()));
    connect(bell, SIGNAL(deviceRemoved(const QString &)), this, SLOT(discEjected(const QString &)));
}

AudioCD::~AudioCD()
{
}

bool AudioCD::setDevice(Solid::Device aCd)
{
    odsign = aCd;
    cd = 0;
    block = 0;
    src = 0;
    ctlr = 0;
    status = NoDisc;
    driveUdi = QString();
    discUdi = QString();

    cdDrive = aCd.as<Solid::OpticalDrive>();
    if (cdDrive) {
        driveUdi = odsign.udi();
        // look for an opticaldisc inserted in this drive
        QList<Solid::Device> devList = Solid::Device::listFromType(Solid::DeviceInterface::OpticalDisc, QString());

        if (!devList.isEmpty()) {
            for (int i = 0; i < devList.size(); ++i) {
                if (devList[i].parentUdi() == odsign.udi()) {
                    cd = devList[i].as<Solid::OpticalDisc>();
                    block = odsign.as<Solid::Block>();
                    src = new MediaSource(Cd, block->device());
                    obj->setCurrentSource(*src);
                    ctlr = new MediaController(obj);
                    discUdi = devList[i].udi();
                    getDiscParameter();
                }
            }
        }
    } else {
        kDebug() << "Drive seems not to be an optical drive!";
        return FALSE;
    }

    return TRUE;
}

Solid::OpticalDrive *AudioCD::getCdDrive() const
{
    return cdDrive;
}

Solid::OpticalDisc *AudioCD::getCd() const
{
    return cd;
}

Phonon::MediaSource *AudioCD::getMediaSource() const
{
    return src;
}

QString AudioCD::getCdPath() const
{
    return block->device();
}

bool AudioCD::isCdInserted() const
{
    return (cd != NULL);
}

bool AudioCD::hasAudio() const
{
    return (cd->availableContent() & Solid::OpticalDisc::Audio) == Solid::OpticalDisc::Audio;
}

AudioCD::DriveStatus AudioCD::getDriveStatus() const
{
    return status;
}

QList<uint> AudioCD::getOffsetList() const
{
    return offsetList;
}

int AudioCD::getDiscLength() const
{
    return discLength;
}

uint AudioCD::getTrackLength(int track) const
{
    return trackLengthList[track];
}

QString AudioCD::getFreeDbId() const
{
    return freeDbId;
}

QString AudioCD::getMusicbrainzId() const
{
    return musicbrainzId;
}

QString AudioCD::getDriveUdi() const
{
    return driveUdi;
}

QString AudioCD::getDiscUdi() const
{
    return discUdi;
}

uint AudioCD::getTrackNum() const
{
    return tracks;
}

void AudioCD::reloadCD()
{
    // look for an opticaldisc inserted in this drive
    QList<Solid::Device> devList = Solid::Device::listFromType(Solid::DeviceInterface::OpticalDisc, QString());

    if (!devList.isEmpty()) {
        for (int i = 0; i < devList.size(); ++i) {
            if (devList[i].parentUdi() == odsign.udi()) {
                kDebug() << "CD inserted!";
                cd = devList[i].as<Solid::OpticalDisc>();
                block = odsign.as<Solid::Block>();
                discUdi = devList[i].udi();
                // cddrives are slow, so give it some seconds to load
                QTimer::singleShot(5000, this, SLOT(registerMediaSource()));
                status = Loading;
                emit driveStatusChanged(status);
            }
        }
    }
}

void AudioCD::eject()
{
    if (cdDrive) cdDrive->eject();
}

void AudioCD::discEjected(const QString &udi)
{
    if (discUdi == udi) {
        cd = 0;
        src = 0;
        tracks = 0;
        offsetList.clear();
        trackLengthList.clear();
        discLength = 0;
        freeDbId = QString();
        musicbrainzId = QString();
        discUdi = QString();
        status = NoDisc;
        emit driveStatusChanged(status);
    }
}
    
QStringList AudioCD::metaData()
{
    QStringList data = obj->metaData(Phonon::MusicBrainzDiscIdMetaData);
    return data;
}

void AudioCD::discInfoChanged()
{
    emit newDiscInfo();
}

Phonon::State AudioCD::state()
{
    return obj->state();
}

void AudioCD::registerMediaSource()
{
    block = odsign.as<Solid::Block>();
    src = new MediaSource(Cd, block->device());
    obj->setCurrentSource(*src);
    ctlr = new MediaController(obj);
    getDiscParameter();
}

void AudioCD::getDiscParameter()
{
    DiscId *discid = discid_new();
    discid_read(discid, (block->device()).toLatin1());
    discLength = discid_get_sectors(discid) * 1000 / 75; // milliseconds
    freeDbId = discid_get_freedb_id(discid);
    musicbrainzId = discid_get_id(discid);
    tracks = discid_get_last_track_num(discid);
    offsetList.clear();
    trackLengthList.clear();
    for (uint i = discid_get_first_track_num(discid); i <= tracks; ++i) {
        offsetList << discid_get_track_offset(discid, i);
        trackLengthList << discid_get_track_length(discid, i) * 1000 / 75;
    }
    offsetList << discid_get_sectors(discid);
    discid_free(discid);

    if (hasAudio()) {
        status = Ready;
        emit driveStatusChanged(Ready);
    } else {
        status = ReadyNoAudio;
        emit driveStatusChanged(status);
    }
}

#include "audiocd.moc"
