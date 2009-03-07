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

#ifndef ENCODERCONFIGIMP_H
#define ENCODERCONFIGIMP_H

#include <QHash>

#include "ui_encoderconfig.h"
#include "ui_encoderedit.h"
#include "encoder_prefs.h"

class EncoderEdit : public QWidget, public Ui::EncoderEdit
{
public:
	EncoderEdit( QWidget *parent ) : QWidget( parent ) {
		setupUi( this );
  }
};


/**
 * Class mainly build to manage encoder list.
 */ 
class EncoderConfigImp : public QWidget, public Ui::EncoderConfig
{

Q_OBJECT

public:
	EncoderConfigImp( QWidget* parent = 0);

private slots:
	void addEncoderSlot();
	void saveNewEncoderSlot(const QString &);
	void removeEncoderSlot();
	void configureEncoderSlot();
	bool checkEncoderName(const QString &);

	void updateEncoder(const QString &);
	void loadEncoderList();
	void setDefaultEncoderSlot();

	void encoderWizard();

signals:
	void encoderChanged();
};

#endif

