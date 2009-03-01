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

#include "prefs.h"
#include "encoder_prefs.h"
#include "encodefileimp.h"
#include "job.h"

//#include <kurlrequester.h>
#include <kmessagebox.h>
#include <kfiledialog.h>
#include <kcombobox.h>
#include <knuminput.h>

#include <QStringList>
#include <QTreeWidget>
#include <QDate>

#include <kdebug.h>

EncodeFileImp::EncodeFileImp(QWidget* parent) : KDialog(parent), m_genres(KCDDB::Genres()), editedItem(0), editedColumn(0)
{
	QWidget *w = new QWidget();
	setupUi(w);
	setMainWidget(w);
	setCaption(i18n("Encode Files"));
	setButtons(User1|Close);
	setButtonText(User1, i18n("&Add to queue"));
	year->setMinimum(999);
	year->setMaximum(QDate::currentDate().year());
	year->setSpecialValueText(i18n("empty"));

	genre->addItems(m_genres.i18nList());
	//   // Specify to only accept wav files
	//   file->setFilter("*.wav|Wav Files");
	//   file->setMode(KFile::File|KFile::ExistingOnly|KFile::LocalOnly);

	QStringList encoderList = EncoderPrefs::prefsList(), encoderNamesList;
	foreach (QString encoderName, encoderList) {
		encoderNamesList << encoderName.remove("Encoder_");
	}
	encoder->addItems(encoderNamesList);

	connect(fileList, SIGNAL(itemDoubleClicked(QTreeWidgetItem *, int)), this, SLOT(editFile(QTreeWidgetItem *, int)));
	connect(fileList, SIGNAL(itemSelectionChanged()), this, SLOT(closeEditor()));
	connect(addFilesButton, SIGNAL(clicked()), this, SLOT(addFiles()));
	connect(clearButton, SIGNAL(clicked()), this, SLOT(clearFileList()));
	connect(removeSelectedButton, SIGNAL(clicked()), this, SLOT(removeSelectedFiles()));

	connect(assignArtistButton, SIGNAL(clicked()), this, SLOT(assignArtist()));
	connect(assignAlbumButton, SIGNAL(clicked()), this, SLOT(assignAlbum()));
	connect(assignYearButton, SIGNAL(clicked()), this, SLOT(assignYear()));
	connect(assignCommentButton, SIGNAL(clicked()), this, SLOT(assignComment()));
	connect(assignGenreButton, SIGNAL(clicked()), this, SLOT(assignGenre()));
	connect(assignEncoderButton, SIGNAL(clicked()), this, SLOT(assignEncoder()));

	connect(this, SIGNAL(user1Clicked()), this, SLOT(encode()));
}

void EncodeFileImp::addFiles()
{
	QString defaultEncoder = Prefs::defaultEncoder();
	QStringList encoderList = EncoderPrefs::prefsList(), encoderNamesList;
	foreach (QString encoderName, encoderList) {
		encoderNamesList << encoderName.remove("Encoder_");
	}
	int indexDefaultEncoder = encoderNamesList.indexOf(defaultEncoder);

	QStringList files = KFileDialog::getOpenFileNames(KUrl(), "*.wav|Wav Files", this, QString());
	foreach (QString track, files) {
		QTreeWidgetItem *newFile = new QTreeWidgetItem(QStringList(track));
		fileList->addTopLevelItem(newFile);
		newFile->setText(COLUMN_TITLE, i18n("track_%1").arg(QString::number(fileList->topLevelItemCount())));
		newFile->setText(COLUMN_ARTIST, i18n("unknown"));
		newFile->setText(COLUMN_ALBUM, i18n("unknown"));

		KComboBox *genreBox = new KComboBox;
		genreBox->addItems(m_genres.i18nList());
		fileList->setItemWidget(newFile, COLUMN_GENRE, genreBox);

		KIntNumInput *yearInput = new KIntNumInput;
		yearInput->setMinimum(999);
		yearInput->setMaximum(QDate::currentDate().year());
		yearInput->setSpecialValueText(i18n("empty"));
		yearInput->setSliderEnabled(true);
		fileList->setItemWidget(newFile, COLUMN_YEAR, yearInput);

		newFile->setText(COLUMN_TRACK, QString::number(fileList->topLevelItemCount()));

		KComboBox *encoderBox = new KComboBox;
		encoderBox->addItems(encoderNamesList);
		encoderBox->setCurrentIndex(indexDefaultEncoder);
		fileList->setItemWidget(newFile, COLUMN_ENCODER, encoderBox);
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
		item->setText(COLUMN_ARTIST, artist->text());
	}
}

void EncodeFileImp::assignAlbum()
{
	foreach (QTreeWidgetItem *item, fileList->selectedItems()) {
		item->setText(COLUMN_ALBUM, album->text());
	}
}

void EncodeFileImp::assignComment()
{
	foreach (QTreeWidgetItem *item, fileList->selectedItems()) {
		item->setText(COLUMN_COMMENT, comment->text());
	}
}

void EncodeFileImp::assignGenre()
{
	foreach (QTreeWidgetItem *item, fileList->selectedItems()) {
		KComboBox *input = (KComboBox *)fileList->itemWidget(item, COLUMN_GENRE);
		input->setCurrentIndex(genre->currentIndex());
	}
}

void EncodeFileImp::assignYear()
{
	foreach (QTreeWidgetItem *item, fileList->selectedItems()) {
		KIntNumInput *input = (KIntNumInput *)fileList->itemWidget(item, COLUMN_YEAR);
		input->setValue(year->value());
	}
}

void EncodeFileImp::assignEncoder()
{
	foreach (QTreeWidgetItem *item, fileList->selectedItems()) {
		KComboBox *input = (KComboBox *)fileList->itemWidget(item, COLUMN_ENCODER);
		input->setCurrentIndex(encoder->currentIndex());
	}
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
	int counter(0);
	QTreeWidgetItemIterator it(fileList);
	while (*it) {
		Job *newJob = new Job();

		newJob->location = (*it)->text(COLUMN_FILE);
		newJob->track_title = (*it)->text(COLUMN_TITLE);
		newJob->track_artist = (*it)->text(COLUMN_ARTIST);
		newJob->album = (*it)->text(COLUMN_ALBUM);
		newJob->track_comment = (*it)->text(COLUMN_COMMENT);
		KIntNumInput *input = (KIntNumInput *)fileList->itemWidget(*it, COLUMN_YEAR);
		newJob->year = input->value();
		KComboBox *box = (KComboBox *)fileList->itemWidget(*it, COLUMN_ENCODER);
		newJob->encoder = box->currentText();
		if(newJob->genre.isEmpty())
		newJob->genre = "Pop";
		newJob->track = ((*it)->text(COLUMN_TRACK)).toInt();
		newJob->removeTempFile = false;

		emit(startJob(newJob));
		++counter;
		++it;
	}

	// Same message and *strings* from tracksimp.cpp
	KMessageBox::information(this,
	i18n("%1 Job(s) have been started.  You can watch their progress in the " \
		"jobs section.", counter),
	i18n("Jobs have started"), i18n("Jobs have started"));
}


#include "encodefileimp.moc"
