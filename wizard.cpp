#include "wizard.h"
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

#include <QString>
#include <QRegExp>
#include <QMap>
#include <klocale.h>
#include "job.h"

fileWizard::fileWizard( QWidget* parent )
    : QDialog( parent ), Ui_fileWizard()
{
    setupUi( this );
    connect(buttonOk, SIGNAL(clicked()), this, SLOT(accept()));
    connect(buttonCancel, SIGNAL(clicked()), this, SLOT(reject()));
    connect(homeDirButton, SIGNAL(clicked()),this, SLOT(homePressed()));
    connect(artistButton, SIGNAL(clicked()), this, SLOT(artistPressed()));
    connect(trackNumberButton, SIGNAL(clicked()), this, SLOT(trackNumberPressed()));
    connect(yearButton, SIGNAL(clicked()), this, SLOT(yearPressed()));
    connect(extensionButton, SIGNAL(clicked()), this, SLOT(extensionPressed()));
    connect(albumButton, SIGNAL(clicked()), this, SLOT(albumPressed()));
    connect(trackTitleButton, SIGNAL(clicked()), this, SLOT(trackTitlePressed()));
    connect(genreButton, SIGNAL(clicked()), this, SLOT(genrePressed()));
    connect(fileFormat, SIGNAL(textChanged(const QString&)), this, SLOT(fileFormatTextChanged(const QString&)));
    connect(trackArtistButton, SIGNAL(clicked()), this, SLOT(trackArtistPressed()));
    connect(trackCommentButton,SIGNAL(clicked()), this, SLOT(trackCommentPressed()));
    connect(commentButton, SIGNAL(clicked()), this, SLOT(commentPressed()));

}

void fileWizard::homePressed()
{
	fileFormat->insert("~");
}

void fileWizard::extensionPressed()
{
	fileFormat->insert("%{extension}");
}

void fileWizard::trackTitlePressed()
{
	fileFormat->insert("%{title}");
}

void fileWizard::trackArtistPressed()
{
	fileFormat->insert("%{artist}");
}

void fileWizard::trackNumberPressed()
{
	fileFormat->insert("%{number}");
}

void fileWizard::trackCommentPressed()
{
	fileFormat->insert("%{comment}");
}

void fileWizard::yearPressed()
{
	fileFormat->insert("%{year}");
}

void fileWizard::genrePressed()
{
	fileFormat->insert("%{genre}");
}

void fileWizard::albumPressed()
{
	fileFormat->insert("%{albumtitle}");
}

void fileWizard::artistPressed()
{
	fileFormat->insert("%{albumartist}");
}

void fileWizard::commentPressed()
{
	fileFormat->insert("%{albumcomment}");
}

void fileWizard::fileFormatTextChanged(const QString& text)
{
	QString string = text;
	string.replace(QRegExp("~"), "/home/foo");
	Job job;
	job.genre = "Rock";
	job.group = "J Rocker";
	job.album = "Why Rain";
	job.year = 2002;
	job.track = 9;
	job.comment = "This Album rocks!";
 
	job.track_title = "Time";
	job.track_artist = "John Rocker"; 
	job.track_comment = "This Song Rocks!";
	QHash<QString,QString> map;
	map.insert("extension", "mp3");
	string = job.replaceSpecialChars(string, false, map);
	exampleLabel->setText(i18n("Example: %1", string));
}

#include "wizard.moc"
