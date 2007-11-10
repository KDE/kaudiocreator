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

#ifndef KAUDIOCREATOR_H
#define KAUDIOCREATOR_H

#include <kxmlguiwindow.h>
#include <kconfigdialog.h>

#include "ui_ripconfig.h"
#include "ui_cdconfig.h"

class KPageWidget;
class TracksImp;
class JobQueImp;
class Ripper;
class Encoder;
class KCModule;
class EncoderConfigImp;
class KMenu;

class CdCfg : public QWidget, public Ui::CdConfig
{
    public:
        CdCfg(QWidget *parent = 0) : QWidget(parent) {
            setupUi(this);
        }
};

class RipCfg : public QWidget, public Ui::RipConfig
{
    public:
        RipCfg(QWidget *parent = 0) : QWidget(parent) {
            setupUi(this);
        }
};

class KAudioCreator : public KXmlGuiWindow {

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
  void showCurrentEncoder();
  void configureNotifications();
  void encodeFile();

  void slotRipSelection(QAction *);
  void setupRipMenu();

private:
  KPageWidget *pageWidget;
  TracksImp   *tracks;
  JobQueImp   *jobQue;
  Ripper      *ripper;
  Encoder     *encoder;
  KMenu       *ripMenu;
  QLabel *defaultEncLabel;
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

