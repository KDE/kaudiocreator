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

#ifndef ENCODEFILEIMP_H
#define ENCODEFILEIMP_H

#include "encodefile.h"
#include "qmap.h"

class Job;

/**
 * This class lets the user encode a file.
 */
class EncodeFileImp : public EncodeFile  {

Q_OBJECT

signals:
  // Start encoding this wav file
  void startJob(Job *newJob);

public:
  EncodeFileImp( QMap<QString, QString> genres, QWidget* parent = 0, const char* name = 0);

protected slots:
  // Encode button
  void encode();

private:
  // List of genres and i18n versions
  QMap<QString, QString> genres;

};

#endif // ENCODEFILEIMP_H

