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

#include "config-kaudiocreator.h"

#include "prefs.h"
#include "encoder_prefs.h"
#include "encodefileimp.h"
#include "job.h"

//#include <kurlrequester.h>
#include <kmessagebox.h>
#include <kfiledialog.h>
#include <kcombobox.h>
#include <knuminput.h>
#include <kmimetype.h>

#include <QStringList>
#include <QTreeWidget>
#include <QDate>
#include <QSet>
#include <QDir>
#include <QDirIterator>

#ifdef HAVE_TAGLIB
#define Qt4QStringToTString(s) TagLib::String(s.toUtf8().data(), TagLib::String::UTF8) 
#include <tstring.h>
#include <tag.h>
#include <fileref.h>
#endif

#include <kdebug.h>

EncodeFileImp::EncodeFileImp(QWidget* parent) : KDialog(parent), editedItem(0), editedColumn(0)
{
	QWidget *w = new QWidget();
	setupUi(w);
	setMainWidget(w);
	setCaption(i18n("Encode Files"));
	setButtons(User1|User2|Close);
	setButtonText(User1, i18n("&Add to queue"));
	setButtonText(User2, i18n("&Add to queue and close"));
	yearInput->setMinimum(EMPTY_YEAR);
	yearInput->setMaximum(QDate::currentDate().year());
	yearInput->setSpecialValueText(i18n("empty"));

	setupGlobals();

	genreBox->addItems(m_genres);

	restoreDialogSize(KConfigGroup(KGlobal::config(), "size_encodefiledialog"));

	connect(fileList, SIGNAL(itemDoubleClicked(QTreeWidgetItem *, int)), this, SLOT(editFile(QTreeWidgetItem *, int)));
	connect(fileList, SIGNAL(itemSelectionChanged()), this, SLOT(closeEditor()));
	connect(fileList, SIGNAL(itemSelectionChanged()), this, SLOT(setupEncoderBox()));
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
void EncodeFileImp::setupEncoderBox()
{
	QStringList fileTypeList, encoders;
	foreach (QTreeWidgetItem *item, fileList->selectedItems()) {
		QString mimeType = KMimeType::extractKnownExtension(item->text(COLUMN_FILE));
		fileTypeList << mimeType;
		encoders << encoderMap[mimeType];
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

	// go through the subdirectories, this is really slow for a lot of files
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
		QTreeWidgetItem *newFile = new QTreeWidgetItem(QStringList(track));
		fileList->addTopLevelItem(newFile);

#ifdef HAVE_TAGLIB
		TagLib::String s(Qt4QStringToTString(track));
		TagLib::FileRef f(s.toCString(true));
		if (taglibExtensions.contains(KMimeType::extractKnownExtension(track)) && !f.tag()->isEmpty()) {
			TagLib::String tagString;
			QString qtString;
			tagString = f.tag()->title();
			tagString == TagLib::String::null ? qtString = i18n("track_%1").arg(QString::number(fileList->topLevelItemCount())) : qtString = TStringToQString(tagString);
			newFile->setText(COLUMN_TITLE, qtString);

			tagString = f.tag()->artist();
			tagString == TagLib::String::null ? qtString = i18n("unknown") : qtString = TStringToQString(tagString);
			newFile->setText(COLUMN_ARTIST, qtString);

			tagString = f.tag()->album();
			tagString == TagLib::String::null ? qtString = i18n("unknown") : qtString = TStringToQString(tagString);
			newFile->setText(COLUMN_ALBUM, qtString);

			tagString = f.tag()->comment();
			tagString == TagLib::String::null ? qtString = QString() : qtString = TStringToQString(tagString);
			newFile->setText(COLUMN_COMMENT, qtString);

			KComboBox *itemGenreBox = new KComboBox;
			itemGenreBox->setEditable(true);
			itemGenreBox->addItems(m_genres);
			tagString = f.tag()->genre();
			tagString == TagLib::String::null ? qtString = i18n("unknown") : qtString = TStringToQString(tagString);
			itemGenreBox->setEditText(qtString);
			fileList->setItemWidget(newFile, COLUMN_GENRE, itemGenreBox);

			KIntNumInput *itemYearInput = new KIntNumInput;
			itemYearInput->setMinimum(EMPTY_YEAR);
			itemYearInput->setMaximum(QDate::currentDate().year());
			itemYearInput->setSpecialValueText(i18n("empty"));
			itemYearInput->setSliderEnabled(true);
			uint y = f.tag()->year();
			if (y == 0) y = EMPTY_YEAR;
			fileList->setItemWidget(newFile, COLUMN_YEAR, itemYearInput);
			itemYearInput->setValue(y);

			KIntNumInput *trackNumberInput = new KIntNumInput;
			uint t = f.tag()->track();
			if (t == 0) t = fileList->topLevelItemCount();
			trackNumberInput->setSliderEnabled(true);
			trackNumberInput->setMinimum(EMPTY_TRACK);
			trackNumberInput->setSpecialValueText(i18n("empty"));
			fileList->setItemWidget(newFile, COLUMN_TRACK, trackNumberInput);
			trackNumberInput->setValue(t);
		} else {
			newFile->setText(COLUMN_TITLE, i18n("track_%1").arg(QString::number(fileList->topLevelItemCount())));
			newFile->setText(COLUMN_ARTIST, i18n("unknown"));
			newFile->setText(COLUMN_ALBUM, i18n("unknown"));

			KComboBox *itemGenreBox = new KComboBox;
			itemGenreBox->setEditable(true);
			itemGenreBox->addItems(m_genres);
			fileList->setItemWidget(newFile, COLUMN_GENRE, itemGenreBox);

			KIntNumInput *itemYearInput = new KIntNumInput;
			itemYearInput->setMinimum(EMPTY_YEAR);
			itemYearInput->setMaximum(QDate::currentDate().year());
			itemYearInput->setSpecialValueText(i18n("empty"));
			itemYearInput->setSliderEnabled(true);
			fileList->setItemWidget(newFile, COLUMN_YEAR, itemYearInput);

			KIntNumInput *trackNumberInput = new KIntNumInput;
			trackNumberInput->setSliderEnabled(true);
			trackNumberInput->setMinimum(EMPTY_TRACK);
			trackNumberInput->setSpecialValueText(i18n("empty"));
			fileList->setItemWidget(newFile, COLUMN_TRACK, trackNumberInput);
			trackNumberInput->setValue(fileList->topLevelItemCount());
		}
#else
		newFile->setText(COLUMN_TITLE, i18n("track_%1").arg(QString::number(fileList->topLevelItemCount())));
		newFile->setText(COLUMN_ARTIST, i18n("unknown"));
		newFile->setText(COLUMN_ALBUM, i18n("unknown"));

		KComboBox *itemGenreBox = new KComboBox;
		itemGenreBox->setEditable(true);
		itemGenreBox->addItems(m_genres);
		fileList->setItemWidget(newFile, COLUMN_GENRE, itemGenreBox);

		KIntNumInput *itemYearInput = new KIntNumInput;
		itemYearInput->setMinimum(EMPTY_YEAR);
		itemYearInput->setMaximum(QDate::currentDate().year());
		itemYearInput->setSpecialValueText(i18n("empty"));
		itemYearInput->setSliderEnabled(true);
		fileList->setItemWidget(newFile, COLUMN_YEAR, itemYearInput);

		KIntNumInput *trackNumberInput = new KIntNumInput;
		trackNumberInput->setSliderEnabled(true);
		trackNumberInput->setMinimum(EMPTY_TRACK);
		trackNumberInput->setSpecialValueText(i18n("empty"));
		fileList->setItemWidget(newFile, COLUMN_TRACK, trackNumberInput);
		trackNumberInput->setValue(fileList->topLevelItemCount());
#endif

		KComboBox *itemEncoderBox = new KComboBox;
		QString extension = KMimeType::extractKnownExtension(track);
		if (encoderMap.contains(extension)) {
			itemEncoderBox->addItems(encoderMap[extension]);
		}
		fileList->setItemWidget(newFile, COLUMN_ENCODER, itemEncoderBox);
	}
}

void EncodeFileImp::clearFileList()
{
	fileList->clear();
}

void EncodeFileImp::removeSelectedFiles()
{
	foreach (QTreeWidgetItem *item, fileList->selectedItems()) {
		fileList->takeTopLevelItem(fileList->indexOfTopLevelItem(item));
	}
}

void EncodeFileImp::assignArtist()
{
	foreach (QTreeWidgetItem *item, fileList->selectedItems()) {
		item->setText(COLUMN_ARTIST, artistEdit->text());
	}
}

void EncodeFileImp::assignAlbum()
{
	foreach (QTreeWidgetItem *item, fileList->selectedItems()) {
		item->setText(COLUMN_ALBUM, albumEdit->text());
	}
}

void EncodeFileImp::assignComment()
{
	foreach (QTreeWidgetItem *item, fileList->selectedItems()) {
		item->setText(COLUMN_COMMENT, commentEdit->text());
	}
}

void EncodeFileImp::assignGenre()
{
	foreach (QTreeWidgetItem *item, fileList->selectedItems()) {
		KComboBox *input = (KComboBox *)fileList->itemWidget(item, COLUMN_GENRE);
		input->setEditText(genreBox->currentText());
	}
}

void EncodeFileImp::assignTrack()
{
	int t = trackStart->value();
	foreach (QTreeWidgetItem *item, fileList->selectedItems()) {
		KIntNumInput *input = (KIntNumInput *)fileList->itemWidget(item, COLUMN_TRACK);
		input->setValue(t);
		if (t != EMPTY_TRACK) ++t;
	}
}

void EncodeFileImp::assignYear()
{
	foreach (QTreeWidgetItem *item, fileList->selectedItems()) {
		KIntNumInput *input = (KIntNumInput *)fileList->itemWidget(item, COLUMN_YEAR);
		input->setValue(yearInput->value());
	}
}

void EncodeFileImp::assignEncoder()
{
	foreach (QTreeWidgetItem *item, fileList->selectedItems()) {
		KComboBox *input = (KComboBox *)fileList->itemWidget(item, COLUMN_ENCODER);
		int index = input->findText(encoderBox->currentText());
		input->setCurrentIndex(index);
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
 * open persistent editor for clicked "cell"
 */
void EncodeFileImp::editFile(QTreeWidgetItem *item, int column)
{
	if (column != COLUMN_FILE) {
		if (!editedItem && item) {
			fileList->openPersistentEditor(item, column);
			editedItem = item;
			editedColumn = column;
		} else if (editedItem && item && (editedItem != item)) {
			fileList->closePersistentEditor(editedItem, editedColumn);
			fileList->openPersistentEditor(item, column);
			editedItem = item;
			editedColumn = column;
		} else if (editedItem && item && (editedItem == item)) {
			if (column == editedColumn) {
				fileList->closePersistentEditor(editedItem, editedColumn);
				editedItem = 0;
			} else {
				fileList->closePersistentEditor(editedItem, editedColumn);
				fileList->openPersistentEditor(item, column);
				editedItem = item;
				editedColumn = column;
			}
		}
	}
}

void EncodeFileImp::closeEditor()
{
	if (editedItem) {
		fileList->closePersistentEditor(editedItem, editedColumn);
		editedItem = 0;
	}
}

/**
 * When the user presses the "add to queue" button create a job with all of the current
 * selection options and emit a signal with it.
 */
void EncodeFileImp::encode()
{
	int jobCounter(0);
	QTreeWidgetItemIterator it(fileList);
	while (*it) {
		Job *newJob = new Job();

		newJob->location = (*it)->text(COLUMN_FILE);
		newJob->track_title = (*it)->text(COLUMN_TITLE);
		newJob->track_artist = (*it)->text(COLUMN_ARTIST);
		newJob->album = (*it)->text(COLUMN_ALBUM);
		newJob->track_comment = (*it)->text(COLUMN_COMMENT);

		KComboBox *genreBox = (KComboBox *)fileList->itemWidget(*it, COLUMN_GENRE);
		newJob->genre = genreBox->currentText();

		KIntNumInput *yearInput = (KIntNumInput *)fileList->itemWidget(*it, COLUMN_YEAR);
		newJob->year = yearInput->value();

		KIntNumInput *trackInput = (KIntNumInput *)fileList->itemWidget(*it, COLUMN_TRACK);
		newJob->track = trackInput->value();

		KComboBox *encoderBox = (KComboBox *)fileList->itemWidget(*it, COLUMN_ENCODER);
		newJob->encoder = encoderBox->currentText();

		newJob->removeTempFile = false;

		emit(startJob(newJob));
		++jobCounter;
		++it;
	}

	// Same message and *strings* from tracksimp.cpp
	KMessageBox::information(this,
	i18n("%1 Job(s) have been started.  You can watch their progress in the " \
		"jobs section.", jobCounter),
	i18n("Jobs have started"), i18n("Jobs have started"));
}

void EncodeFileImp::encodeAndClose()
{
	encode();
	accept();
}

#include "encodefileimp.moc"
