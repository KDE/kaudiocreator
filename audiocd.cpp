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
    obj = new MediaObject(this);
    ctlr = 0;
}

AudioCD::AudioCD(Solid::Device aCd)
{
    odsign=aCd;

    bell=Solid::DeviceNotifier::instance();

    // get the opticaldrive
    cdDrive=aCd.as<Solid::OpticalDrive>();

    connect(cdDrive, SIGNAL(ejectDone(Solid::ErrorType, QVariant, const QString &)), this, SLOT(catchEjectPressed()));
    connect(bell, SIGNAL(deviceAdded(const QString)), this, SLOT(reloadCD()));
    connect(bell, SIGNAL(deviceRemoved(const QString)), this, SLOT(reloadCD()));

    cd = 0;
    block = 0;
    src = 0;
    obj = new MediaObject(this);
    connect(obj, SIGNAL(stateChanged(Phonon::State, Phonon::State)), this, SLOT(cdStateChanged(Phonon::State, Phonon::State)));
    connect(obj, SIGNAL(metaDataChanged()), this, SLOT(discInfoChanged()));
    ctlr = 0;

    // look for an opticaldisc inserted in this drive
    QList<Solid::Device> devList = Solid::Device::listFromType(Solid::DeviceInterface::OpticalDisc, QString());

    if (devList.isEmpty()) {
        kDebug() << "No Optical Disc detected in the computer!";
        cd = NULL;
        block = NULL;
        src = NULL;
    } else {
        for (int i = 0; i < devList.size(); ++i) {
            if (devList[i].parentUdi()==odsign.udi()) {
                cd = devList[i].as<Solid::OpticalDisc>();
                block = odsign.as<Solid::Block>();
                src = new MediaSource(Cd, block->device());
                obj->setCurrentSource(*src);
                ctlr = new MediaController(obj);
                getDiscParameter();
            }
        }
    }
}

AudioCD::~AudioCD()
{
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

QList<uint> AudioCD::getOffsetList()
{
    return offsetList;
}

int AudioCD::getDiscLength()
{
    return discLength;
}

uint AudioCD::getTrackLength(int track)
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

QString AudioCD::getSignature() const
{
    return odsign.udi();
}

uint AudioCD::getTrackNum() const
{
    return tracks;
}


void AudioCD::catchEjectPressed()
{
    qDebug() << "catchEjectPressed";
    cd = NULL;
    block = NULL;
    src = NULL;

    emit(discRemoved());
}

void AudioCD::reloadCD()
{
    // look for an opticaldisc inserted in this drive
    QList<Solid::Device> devList = Solid::Device::listFromType(Solid::DeviceInterface::OpticalDisc, QString());

    if (devList.isEmpty()) {
        kDebug() << "No Optical Disc detected in the computer!";
        cd = NULL;
        block = NULL;
        src = NULL;
        emit(discRemoved());
    } else {
        for (int i = 0; i < devList.size(); ++i) {
            if (devList[i].parentUdi() == odsign.udi()) {
                qDebug() << "CD inserted!";
                cd = devList[i].as<Solid::OpticalDisc>();
                block = odsign.as<Solid::Block>();
                // cddrives are slow, so give it some seconds to load
                QTimer::singleShot(5000, this, SLOT(registerMediaSource()));
            }
        }
    }
}

void AudioCD::discEjected()
{
    cd = NULL;
    src = NULL;
    tracks = 0;
    offsetList.clear();
    trackLengthList.clear();
    discLength = 0;
    freeDbId = QString();
    musicbrainzId = QString();
    emit(discRemoved());
}
    
QStringList AudioCD::metaData()
{
    QStringList data = obj->metaData(Phonon::MusicBrainzDiscIdMetaData);
    return data;
}

void AudioCD::cdStateChanged(Phonon::State newState, Phonon::State oldState)
{
/*    qDebug() << newState << oldState;
    if (oldState == Phonon::LoadingState && newState == Phonon::StoppedState) {
        ctlr->setAutoplayTitles(FALSE);
        if (ctlr->currentTitle() <= tracks())
            ctlr->nextTitle();*/
//        qDebug() << "Time:" << obj->totalTime();
//     }
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
    qDebug() << (cd->availableContent() & Solid::OpticalDisc::Audio);
    emit(discInserted());
}

#include "audiocd.moc"
