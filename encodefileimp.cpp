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
#include <kurlrequester.h>
#include <qcombobox.h>
#include <kmessagebox.h>

EncodeFileImp::EncodeFileImp( QMap<QString, QString> g, QWidget* parent, const char* name) : EncodeFile(parent, name), genres(g) {
  genre->insertStringList(genres.keys());
  file->setFilter("*.wav|Wav files");
}

void EncodeFileImp::accept(){
  Job *newJob = new Job();
  newJob->location = file->url();
  newJob->album = album->text();
  newJob->genre = genres[genre->currentText()];
  if(newJob->genre.isEmpty())
     newJob->genre = "Pop";
  newJob->group = artist->text();
  newJob->comment = comment->text();
  newJob->year = year->value();
  newJob->track = track->value();
     
  newJob->song = track_title->text();
  newJob->song_artist = track_artist->text();
  newJob->song_comment = track_comment->text();
  emit(startJob(newJob));
  
  // Same message from tracksimp.cpp
  int counter(1);
  KMessageBox::information(this,
  i18n("%1 Job(s) have been started.  You can watch their progress in the jobs section.").arg(counter),
 i18n("Jobs have started"), i18n("Jobs have started"));

}
