/****************************************************************************
** ui.h extension file, included from the uic-generated form implementation.
**
** If you wish to add, delete or rename slots use Qt Designer which will
** update this file, preserving your code. Create an init() slot in place of
** a constructor, and a destroy() slot in place of a destructor.
*****************************************************************************/

#include <qstring.h>
#include <qregexp.h>

#include <klocale.h>

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
