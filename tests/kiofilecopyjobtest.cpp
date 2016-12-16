/*
 * Copyright (C) 2016 Leslie Zhai <xiangzhai83@gmail.com>
 * Copyright (C) 2011 Michal Malek <michalm@jabster.pl>
 *
 * This file is part of the K3b project.
 * Copyright (C) 1998-2009 Sebastian Trueg <trueg@k3b.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * See the file "COPYING" for the exact licensing terms.
 */

#include "kiofilecopyjobtest.h"

#include <kio/filecopyjob.h>

#include <QtTest/QSignalSpy>
#include <QtTest/QTest>
#include <QStandardPaths>

QTEST_GUILESS_MAIN(FilecopyJobTest)

FilecopyJobTest::FilecopyJobTest()
{
}

void FilecopyJobTest::init()
{
}


void FilecopyJobTest::cleanp()
{
}


void FilecopyJobTest::test()
{
    QString tmpFileName = "/tmp/1_Ludwig van Beethoven-Symphonie No.5 en ut mineur, OP.67 : 1er mouvement_SazGha.wav";
    QUrl dest = QUrl::fromLocalFile(tmpFileName);
    QUrl source = QUrl::fromLocalFile(QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + "/kaudiocreatorrc");
    KIO::FileCopyJob *copyJob = KIO::file_copy(source, dest, 0644, KIO::HideProgressInfo);
    copyJob->exec();
}
