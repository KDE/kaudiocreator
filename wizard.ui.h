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

/****************************************************************************
** ui.h extension file, included from the uic-generated form implementation.
**
** If you wish to add, delete or rename slots use Qt Designer which will
** update this file, preserving your code. Create an init() slot in place of
** a constructor, and a destroy() slot in place of a destructor.
*****************************************************************************/

#include <qstring.h>
#include <qregexp.h>

#include <klocaleh>

void fileWizard::homePressed()
{
    playlistFormat->insert("~");
}


void fileWizard::albumPressed()
{
    playlistFormat->insert("%album");
}

void fileWizard::artistPressed()
{
    playlistFormat->insert("%artist");
}

void fileWizard::extensionPressed()
{
    playlistFormat->insert("%extension");
}

void fileWizard::genrePressed()
{
    playlistFormat->insert("%genre");
}

void fileWizard::songPressed()
{
    playlistFormat->insert("%song");
}

void fileWizard::trackPressed()
{
    playlistFormat->insert("%track");
}

void fileWizard::yearPressed()
{
    playlistFormat->insert("%year");
}


void fileWizard::fileFormatTextChanged(const QString& text)
{
   QString string = text;
    string.replace(QRegExp("%album"), "Why Rain");
    string.replace(QRegExp("%genre"), "Rock");
    string.replace(QRegExp("%artist"), "J Rocker");
    string.replace(QRegExp("%year"), "2002");
    string.replace(QRegExp("%song"), "Time");
    string.replace(QRegExp("%extension"), "mp3");
    string.replace(QRegExp("%track"), "9");
    string.replace(QRegExp("~"), "/home/foo");
    exampleLabel->setText(i18n("Example: %1").arg(string));
}
