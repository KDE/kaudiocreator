/**
 * Copyright (C) 2003-2005 Benjamin C Meyer (ben at meyerhome dot net)
 * Copyright (C) 2009 Gerd Fleischer (gerdfleischer at web dot de)
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

#include "general.h"
#include "job.h"

#include <kmessagebox.h>

#include <QRegExp>

General::General( QWidget* parent )
	: QWidget( parent )
{
	setupUi( this );
	connect(kcfg_replaceInput,SIGNAL(textChanged(const QString &)),this, SLOT(updateExample()));
	connect(kcfg_replaceOutput, SIGNAL(textChanged(const QString &)), this,SLOT(updateExample()));
	connect(example, SIGNAL(textChanged(const QString &)), this, SLOT(updateExample()));
	connect(kcfg_fat32compatible, SIGNAL(toggled(bool)), this, SLOT(updateExample()));
	connect(kcfg_replaceFatIncompatible, SIGNAL(textChanged(const QString &)), this,SLOT(check_fat32_compatibility(const QString &)));
}

void General::updateExample()
{
	QString text = example->text();
	text.replace( QRegExp(kcfg_replaceInput->text()), kcfg_replaceOutput->text() );
	exampleOutput->setText(text);
}

void General::check_fat32_compatibility(const QString &text)
{
	bool containsNonFat = false;
	QString rep = text;
	QStringList wrongParts, list;
	list << "\\" << "/" << ":" << "*" << "?" << "<" << ">" << "|";
	
	foreach (QString nonFat, list) {
		if (rep.contains(nonFat)) {
			containsNonFat = true;
			wrongParts << nonFat;
		}
	}
	
	if (containsNonFat) {
		KMessageBox::information(this, i18n("The replacement string still contains incompatible characters (%1).\nCopying to FAT32 might not work as expected.").arg(wrongParts.join(", ")));
	}
}

#include "general.moc"
