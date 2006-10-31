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

#ifndef KAUDIOCREATOR_H
#define KAUDIOCREATOR_H

#include <kmainwindow.h>
#include <kconfigdialog.h>

class KJanusWidget;
class TracksImp;
class JobQueImp;
class Ripper;
class Encoder;
class KCModule;
class EncoderConfigImp;
class KPopupMenu;

class KAudioCreator : public KMainWindow {

Q_OBJECT

public:
  KAudioCreator( QWidget* parent = 0, const char* name = 0);
  void setDevice( const QString &device );

protected:
  virtual bool queryClose();

private slots:
  void showSettings();
  void updateStatus();

  void hasCD(bool);
  void configureNotifications();
  void encodeFile();

  void slotRipSelection(int);
  void getRipMenu();

private:
  KJanusWidget *janusWidget;
  TracksImp    *tracks;
  JobQueImp    *jobQue;
  Ripper       *ripper;
  Encoder      *encoder;
  KPopupMenu   *ripMenu;

};

class SettingsDialog: public KConfigDialog {
Q_OBJECT

public:
  SettingsDialog(QWidget *parent, const char *name,KConfigSkeleton *config);

protected slots:
  void updateSettings();
  void updateWidgets();
  void updateWidgetsDefault();
  void slotCddbChanged(bool);

protected:
  bool hasChanged();
  bool isDefault();

public:
   EncoderConfigImp *encoderConfigImp;

private:
  KCModule *cddb;
  bool      cddbChanged;
};

#endif

