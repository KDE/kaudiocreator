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

#ifndef ENCODERCONFIGIMP_H
#define ENCODERCONFIGIMP_H

#include "encoderconfig.h"
#include "encoder_prefs.h"
#include <qmap.h>
#include <qdict.h>

/**
 * Class mainly build to manage encoder list.
 */
class EncoderConfigImp : public EncoderConfig {

Q_OBJECT

signals:
  void encoderUpdated();

public:
  EncoderConfigImp( QWidget* parent = 0, const char* name = 0);

private slots:
  void addEncoderSlot();
  void removeEncoderSlot();
  void configureEncoderSlot();

  void updateEncoder(QObject * obj);
  void updateEncoder(const char *dialogName);
  void loadEncoderList();

private:
  // Name, groupName
  QMap<QString, QString> encoderNames;
};

#endif

