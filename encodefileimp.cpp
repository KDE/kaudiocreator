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

#include "config-kaudiocreator.h"

#include "prefs.h"
#include "encoder_prefs.h"
#include "encodefileimp.h"
#include "encodefilemodel.h"
#include "encodefiledelegate.h"
#include "job.h"

//#include <kurlrequester.h>
#include <kmessagebox.h>
#include <kfiledialog.h>
#include <kcombobox.h>
#include <knuminput.h>

#include <QStringList>
#include <QTreeView>
#include <QModelIndex>
#include <QDate>
#include <QSet>
#include <QDir>
#include <QDirIterator>
// #include <QSize>

#ifdef HAVE_TAGLIB
#define Qt4QStringToTString(s) TagLib::String(s.toUtf8().data(), TagLib::String::UTF8)
#include <tstring.h>
#include <tag.h>
#include <fileref.h>
#endif

#include <kdebug.h>

EncodeFileImp::EncodeFileImp(QWidget* parent) : KDialog(parent), editedColumn(0)
{
	QWidget *w = new QWidget();
	setupUi(w);
	setMainWidget(w);
	setCaption(i18n("Encode Files"));
	setButtons(Default|User1|User2|Close);
	setButtonText(Default, i18n("Fit Columns to Content"));
	setButtonIcon(Default, KIcon("resizeimages"));
	setButtonText(User1, i18n("&Add to queue"));
	setButtonText(User2, i18n("&Add to queue and close"));
	yearInput->setMinimum(EMPTY_YEAR);
	yearInput->setMaximum(QDate::currentDate().year());
	yearInput->setSpecialValueText(i18n("empty"));

	setupGlobals();
    fileListModel = new EncodeFileModel(this);
    fileListView->setModel(fileListModel);
    fileListView->setItemDelegate(new EncodeFileDelegate());
	genreBox->addItems(m_genres);

	restoreDialogSize(KConfigGroup(KGlobal::config(), "size_encodefiledialog"));

	connect(fileListView->selectionModel(), SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection &)), this, SLOT(setupEncoderBox(const QItemSelection &, const QItemSelection &)));
	connect(addFilesButton, SIGNAL(clicked()), this, SLOT(openFiles()));
	connect(addDirectoryButton, SIGNAL(clicked()), this, SLOT(openDirectory()));
	connect(clearButton, SIGNAL(clicked()), this, SLOT(clearFileList()));
	connect(removeSelectedButton, SIGNAL(clicked()), this, SLOT(removeSelectedFiles()));

	connect(assignArtistButton, SIGNAL(clicked()), this, SLOT(assignArtist()));
	connect(assignAlbumButton, SIGNAL(clicked()), this, SLOT(assignAlbum()));
	connect(assignYearButton, SIGNAL(clicked()), this, SLOT(assignYear()));
	connect(assignCommentButton, SIGNAL(clicked()), this, SLOT(assignComment()));
	connect(assignGenreButton, SIGNAL(clicked()), this, SLOT(assignGenre()));
	connect(assignTrackButton, SIGNAL(clicked()), this, SLOT(assignTrack()));
	connect(assignEncoderButton, SIGNAL(clicked()), this, SLOT(assignEncoder()));
	connect(assignAllButton, SIGNAL(clicked()), this, SLOT(assignAll()));

	connect(this, SIGNAL(defaultClicked()), this, SLOT(fitToContent()));
	connect(this, SIGNAL(user1Clicked()), this, SLOT(encode()));
	connect(this, SIGNAL(user2Clicked()), this, SLOT(encodeAndClose()));

	connect(this, SIGNAL(user2Clicked()), this, SLOT(saveSize()));
	connect(this, SIGNAL(closeClicked()), this, SLOT(saveSize()));
}

void EncodeFileImp::saveSize()
{
	KConfigGroup group(KGlobal::config(), "size_encodefiledialog");
	saveDialogSize(group);
	group.sync();
}

/**
 * setup the filter for openFilesDialog
 * setup a map for filetypes and encoders
 */
void EncodeFileImp::setupGlobals()
{
	m_genres = KCDDB::Genres().i18nList();

	dirFilter.clear();
	QString filter;
	QStringList supportedInputTypes = Prefs::inputTypesList();
	// one for every supported filetype
	foreach (QString type, supportedInputTypes) {
		filter.append("*." + type + " ");
		dirFilter << QString("*." + type);
	}
	filter.remove(filter.length()-1, 1);
	filter.append("|All supported types\n");
	// and each filetype separate
	foreach (QString type, supportedInputTypes) {
		filter.append("*." + type + "\n");
	}
	filter.remove(filter.length()-1, 1);
	fileTypeFilter = filter;

	encoderMap.clear();
	foreach (QString type, Prefs::inputTypesList()) {
		foreach (QString encoder, EncoderPrefs::prefsList()) {
			QStringList encoderInputTypesList = (EncoderPrefs::prefs(encoder)->inputTypes()).split(",", QString::SkipEmptyParts);
			if (encoderInputTypesList.contains(type)) {
				if (encoderMap.contains(type)) {
					(encoderMap[type]).append(encoder.remove("Encoder_"));
				} else {
					QStringList enc(encoder.remove("Encoder_"));
					encoderMap[type] = enc;
				}
			}
		}
	}

#ifdef HAVE_TAGLIB
    TagLib::StringList extensions = TagLib::FileRef::defaultFileExtensions();
    TagLib::String tagExt = extensions.toString(",");
    QString qtExt = TStringToQString(tagExt);
    taglibExtensions = qtExt.split(",");
#endif
}

/**
 * setup/update encoderBox
 */
void EncodeFileImp::setupEncoderBox(const QItemSelection &selected, const QItemSelection &)
{
    QStringList fileTypeList, encoders;
    QModelIndex index;
    QModelIndexList items = selected.indexes();
    foreach (index, items) {
        if (index.column() == COLUMN_ENCODER) {
            QString mimeType = fileListModel->data(index, Qt::UserRole).toString();
            fileTypeList << mimeType;
            encoders << encoderMap[mimeType];
        }
	}

	foreach (QString enc, encoders) {
		foreach (QString type, fileTypeList) {
			if (!(encoderMap[type]).contains(enc))
				encoders.removeAll(enc);
		}
	}

	QSet<QString> encodersSet = encoders.toSet(); // remove duplicates
	encoders = encoders.fromSet(encodersSet);
	encoders.sort();
	encoderBox->clear();
	encoderBox->addItems(encoders);
}

void EncodeFileImp::openFiles()
{
	QStringList files = KFileDialog::getOpenFileNames(KUrl(), fileTypeFilter, this, QString());
	addFilesToList(files);
}

void EncodeFileImp::openDirectory()
{
	QStringList files;
	QString dirPath = KFileDialog::getExistingDirectory(KUrl(), this, QString());
	QDir dir(dirPath);
	
	QStringList entries = dir.entryList(dirFilter, QDir::Files);
 	foreach (QString file, entries) {
 		files << dir.absoluteFilePath(file);
 	}

	// go through the subdirectories, this is really slow for a lot of files, use KDirLister?
	if (!(dir.entryList(QDir::AllDirs|QDir::NoDotAndDotDot)).isEmpty()) {
		QDirIterator it(dirPath, QDir::AllDirs|QDir::NoDotAndDotDot, QDirIterator::Subdirectories);
		while (it.hasNext()) {
			dir = QDir(it.next());
			entries = dir.entryList(dirFilter, QDir::Files);
			foreach (QString file, entries) {
				files << dir.absoluteFilePath(file);
			}
		}
	}

	addFilesToList(files);
}

void EncodeFileImp::addFilesToList(const QStringList &list)
{
    foreach (QString track, list) {

        QList<QStandardItem *> fileItems = QList<QStandardItem *>();

        QStandardItem *pathItem = new QStandardItem(track);
        pathItem->setEditable(FALSE);
        fileItems << pathItem;

#ifdef HAVE_TAGLIB
        TagLib::String s(Qt4QStringToTString(track));
        TagLib::FileRef f(s.toCString(true));
        if (taglibExtensions.contains(KMimeType::extractKnownExtension(track)) && !f.tag()->isEmpty()) {
            TagLib::String tagString;
            QString qtString;

            tagString = f.tag()->title();
            tagString == TagLib::String::null ? qtString = i18n("track_%1").arg(QString::number(fileListModel->rowCount() + 1)) : qtString = TStringToQString(tagString);
            fileItems << new QStandardItem(qtString); // title

            tagString = f.tag()->artist();
            tagString == TagLib::String::null ? qtString = i18n("unknown") : qtString = TStringToQString(tagString);
            fileItems << new QStandardItem(qtString); // artist

            tagString = f.tag()->album();
            tagString == TagLib::String::null ? qtString = i18n("unknown") : qtString = TStringToQString(tagString);
            fileItems << new QStandardItem(qtString); // album

            tagString = f.tag()->comment();
            tagString == TagLib::String::null ? qtString = QString() : qtString = TStringToQString(tagString);
            fileItems << new QStandardItem(qtString); // comment

            tagString = f.tag()->genre();
            tagString == TagLib::String::null ? qtString = i18n("unknown") : qtString = TStringToQString(tagString);
            QStandardItem *genreItem = new QStandardItem(qtString);
            genreItem->setData(QVariant(m_genres), Qt::UserRole);
            fileItems << genreItem;

            uint t = f.tag()->track();
            t == 0 ? qtString = QString::number(fileListModel->rowCount() + 1) : qtString = QString::number(t);
            fileItems << new QStandardItem(qtString); // track number

            uint y = f.tag()->year();
            y == 0 ? qtString = QString() : qtString = QString::number(y);
            fileItems << new QStandardItem(qtString); // year
        } else {
            fileItems << new QStandardItem(i18n("track_%1").arg(QString::number(fileListModel->rowCount() + 1))); // title
            fileItems << new QStandardItem(i18n("unknown")); // artist
            fileItems << new QStandardItem(i18n("unknown")); // album
            fileItems << new QStandardItem(QString()); // comment
            QStandardItem *genreItem = new QStandardItem(QString());
            genreItem->setData(QVariant(m_genres), Qt::UserRole);
            fileItems << genreItem;
            fileItems << new QStandardItem(QString::number(fileListModel->rowCount() + 1)); // track number
            fileItems << new QStandardItem(QString()); // year
        }
#else
        fileItems << new QStandardItem(i18n("track_%1").arg(QString::number(fileListModel->rowCount() + 1))); // title
        fileItems << new QStandardItem(i18n("unknown")); // artist
        fileItems << new QStandardItem(i18n("unknown")); // album
        fileItems << new QStandardItem(QString()); // comment
        QStandardItem *genreItem = new QStandardItem(QString());
        genreItem->setData(QVariant(m_genres), Qt::UserRole);
        fileItems << genreItem;
        fileItems << new QStandardItem(QString::number(fileListModel->rowCount() + 1)); // track number
        fileItems << new QStandardItem(QString()); // year
#endif

        QString extension = KMimeType::extractKnownExtension(track);
        QString encoder = QString();
        if (encoderMap[extension].contains(Prefs::defaultEncoder())) {
            encoder = Prefs::defaultEncoder();
        } else {
            encoder = encoderMap[extension].at(0);
        }
        QStandardItem *encoderItem = new QStandardItem(encoder);
        encoderItem->setData(extension, Qt::UserRole);
        encoderItem->setData(encoderMap[extension], Qt::UserRole + 1);
        fileItems << encoderItem; // encoder

        fileListModel->appendRow(fileItems);
	} // foreach
}

void EncodeFileImp::fitToContent()
{
	for (int c = 0; c < fileListModel->columnCount(); ++c) {
		fileListView->resizeColumnToContents(c);
	}
}

void EncodeFileImp::clearFileList()
{
	fileListModel->clear();
}

void EncodeFileImp::removeSelectedFiles()
{
    QModelIndex index;
    QModelIndexList items = fileListView->selectionModel()->selectedRows(0);
    foreach (index, items) {
        fileListModel->removeRow(index.row(), QModelIndex());
    }
}

void EncodeFileImp::assignArtist()
{
    assignItemText(COLUMN_ARTIST, artistEdit->text());
}

void EncodeFileImp::assignAlbum()
{
    assignItemText(COLUMN_ALBUM, albumEdit->text());
}

void EncodeFileImp::assignComment()
{
    assignItemText(COLUMN_COMMENT, commentEdit->text());
}

void EncodeFileImp::assignGenre()
{
    assignItemText(COLUMN_GENRE, genreBox->currentText());
}

void EncodeFileImp::assignTrack()
{
    int t = trackStart->value();
    QModelIndex index;
    QModelIndexList items = fileListView->selectionModel()->selectedRows(COLUMN_TRACK);
    foreach (index, items) {
        fileListModel->setData(index, QString::number(t), Qt::DisplayRole);
        ++t;
    }
}

void EncodeFileImp::assignYear()
{
    assignItemText(COLUMN_YEAR, QString::number(yearInput->value()));
}

void EncodeFileImp::assignEncoder()
{
    assignItemText(COLUMN_ENCODER, encoderBox->currentText());
}

void EncodeFileImp::assignItemText(int column, const QString &text)
{
    QModelIndex index;
    QModelIndexList items = fileListView->selectionModel()->selectedRows(column);
    foreach (index, items) {
        fileListModel->setData(index, text, Qt::DisplayRole);
    }
}

void EncodeFileImp::assignAll()
{
	assignArtist();
	assignAlbum();
	assignComment();
	assignGenre();
	assignTrack();
	assignYear();
	assignEncoder();
}

/**
 * When the user presses the "add to queue" button create a job with all of the current
 * selection options and emit a signal with it.
 */
void EncodeFileImp::encode()
{
	int jobCounter = 0;

    int rows = fileListModel->rowCount();
    for (int r = 0; r < rows; ++r) {
		Job *newJob = new Job();
 
		newJob->location = fileListModel->data(fileListModel->index(r, COLUMN_FILE, QModelIndex()), Qt::DisplayRole).toString();
		newJob->track_title = fileListModel->data(fileListModel->index(r, COLUMN_TITLE, QModelIndex()), Qt::DisplayRole).toString();
		newJob->track_artist = fileListModel->data(fileListModel->index(r, COLUMN_ARTIST, QModelIndex()), Qt::DisplayRole).toString();
		newJob->album = fileListModel->data(fileListModel->index(r, COLUMN_ALBUM, QModelIndex()), Qt::DisplayRole).toString();
		newJob->track_comment = fileListModel->data(fileListModel->index(r, COLUMN_COMMENT, QModelIndex()), Qt::DisplayRole).toString();
        newJob->genre = fileListModel->data(fileListModel->index(r, COLUMN_GENRE, QModelIndex()), Qt::DisplayRole).toString();
		newJob->year = fileListModel->data(fileListModel->index(r, COLUMN_YEAR, QModelIndex()), Qt::DisplayRole).toInt();
		newJob->track = fileListModel->data(fileListModel->index(r, COLUMN_TRACK, QModelIndex()), Qt::DisplayRole).toInt();
		newJob->encoder = fileListModel->data(fileListModel->index(r, COLUMN_ENCODER, QModelIndex()), Qt::DisplayRole).toString();

		newJob->removeTempFile = false;

		emit(startJob(newJob));
		++jobCounter;
	}

	// Same message and *strings* from tracksimp.cpp
	KMessageBox::information(this,
	i18n("%1 Job(s) have been started.  You can watch their progress in the " \
		"jobs section.", jobCounter),
	i18n("Jobs have started"), i18n("Jobs have started"));

	if (jobCounter > 0) emit(allJobsStarted());
}

void EncodeFileImp::encodeAndClose()
{
	encode();
	accept();
}

#include "encodefileimp.moc"
