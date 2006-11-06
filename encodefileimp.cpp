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

#include "encodefileimp.h"
#include "job.h"

#include <qspinbox.h>
#include <qlineedit.h>
#include <kurlrequester.h>
#include <qcombobox.h>
#include <kmessagebox.h>

EncodeFileImp::EncodeFileImp(QWidget* parent,
		const char* name) : EncodeFile(parent, name), m_genres(KCDDB::Genres()) {
  genre->insertStringList(m_genres.i18nList());
  // Specify to only accept wav files
  file->setFilter("*.wav|Wav Files");

  connect(file,SIGNAL(textChanged(const QString &)),this,SLOT(enableEncodeButton(const QString &)));
  connect(encodeButton,SIGNAL(clicked()),this,SLOT(encode()));
}

/**
 * When the user presses the encode button create a job with all of the current
 * selection options and emit a signal with it.
 */
void EncodeFileImp::encode(){
  Job *newJob = new Job();

  newJob->location = file->url();

  newJob->album = album->text();
  newJob->genre = m_genres.i18n2cddb(genre->currentText());
  if(newJob->genre.isEmpty())
     newJob->genre = "Pop";
  newJob->group = artist->text();
  newJob->comment = comment->text();
  newJob->year = year->value();
  newJob->track = track->value();

  newJob->track_title = track_title->text();
  if ((track_artist->text()).isEmpty())
     newJob->track_artist = artist->text();
  else
  newJob->track_artist = track_artist->text();
  newJob->track_comment = track_comment->text();

  newJob->removeTempFile = false;

  emit(startJob(newJob));

  // Same message and *strings* from tracksimp.cpp
  int counter(1);
  KMessageBox::information(this,
  i18n("%1 Job(s) have been started.  You can watch their progress in the " \
	  "jobs section.").arg(counter),
 i18n("Jobs have started"), i18n("Jobs have started"));
}

void EncodeFileImp::enableEncodeButton(const QString &text){
  encodeButton->setEnabled(!text.isEmpty());
}


#include "encodefileimp.moc"
