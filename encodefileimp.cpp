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
		const char* name) : EncodeFile(parent, name) {
  // DON't i18n this! It is to catch stupid people who don't bother
  // setting the cddbInfo.genre and encoders that barf on empty strings for
  // the cddbInfo.genre (e.g. lame).
  genres.insert("Unknown", "Pop");

  genres.insert(i18n("A Cappella"), "A Cappella");
  genres.insert(i18n("Acid Jazz"), "Acid Jazz");
  genres.insert(i18n("Acid Punk"), "Acid Punk");
  genres.insert(i18n("Acid"), "Acid");
  genres.insert(i18n("Acoustic"), "Acoustic");
  genres.insert(i18n("Alternative"), "Alternative");
  genres.insert(i18n("Alt. Rock"), "Alt. Rock");
  genres.insert(i18n("Ambient"), "Ambient");
  genres.insert(i18n("Anime"), "Anime");
  genres.insert(i18n("Avantgarde"), "Avantgarde");
  genres.insert(i18n("Ballad"), "Ballad");
  genres.insert(i18n("Bass"), "Bass");
  genres.insert(i18n("Beat"), "Beat");
  genres.insert(i18n("Bebop"), "Bebop");
  genres.insert(i18n("Big Band"), "Big Band");
  genres.insert(i18n("Black Metal"), "Black Metal");
  genres.insert(i18n("Bluegrass"), "Bluegrass");
  genres.insert(i18n("Blues"), "Blues");
  genres.insert(i18n("Booty Bass"), "Booty Bass");
  genres.insert(i18n("BritPop"), "BritPop");
  genres.insert(i18n("Cabaret"), "Cabaret");
  genres.insert(i18n("Celtic"), "Celtic");
  genres.insert(i18n("Chamber Music"), "Chamber Music");
  genres.insert(i18n("Chanson"), "Chanson");
  genres.insert(i18n("Chorus"), "Chorus");
  genres.insert(i18n("Christian Gangsta Rap"), "Christian Gangsta Rap");
  genres.insert(i18n("Christian Rap"), "Christian Rap");
  genres.insert(i18n("Christian Rock"), "Christian Rock");
  genres.insert(i18n("Classical"), "Classical");
  genres.insert(i18n("Classic Rock"), "Classic Rock");
  genres.insert(i18n("Club-house"), "Club-house");
  genres.insert(i18n("Club"), "Club");
  genres.insert(i18n("Comedy"), "Comedy");
  genres.insert(i18n("Contemporary Christian"), "Contemporary Christian");
  genres.insert(i18n("Country"), "Country");
  genres.insert(i18n("Crossover"), "Crossover");
  genres.insert(i18n("Cult"), "Cult");
  genres.insert(i18n("Dance Hall"), "Dance Hall");
  genres.insert(i18n("Dance"), "Dance");
  genres.insert(i18n("Darkwave"), "Darkwave");
  genres.insert(i18n("Death Metal"), "Death Metal");
  genres.insert(i18n("Disco"), "Disco");
  genres.insert(i18n("Dream"), "Dream");
  genres.insert(i18n("Drum & Bass"), "Drum & Bass");
  genres.insert(i18n("Drum Solo"), "Drum Solo");
  genres.insert(i18n("Duet"), "Duet");
  genres.insert(i18n("Easy Listening"), "Easy Listening");
  genres.insert(i18n("Electronic"), "Electronic");
  genres.insert(i18n("Ethnic"), "Ethnic");
  genres.insert(i18n("Eurodance"), "Eurodance");
  genres.insert(i18n("Euro-House"), "Euro-House");
  genres.insert(i18n("Euro-Techno"), "Euro-Techno");
  genres.insert(i18n("Fast-Fusion"), "Fast-Fusion");
  genres.insert(i18n("Folklore"), "Folklore");
  genres.insert(i18n("Folk/Rock"), "Folk/Rock");
  genres.insert(i18n("Folk"), "Folk");
  genres.insert(i18n("Freestyle"), "Freestyle");
  genres.insert(i18n("Funk"), "Funk");
  genres.insert(i18n("Fusion"), "Fusion");
  genres.insert(i18n("Game"), "Game");
  genres.insert(i18n("Gangsta Rap"), "Gangsta Rap");
  genres.insert(i18n("Goa"), "Goa");
  genres.insert(i18n("Gospel"), "Gospel");
  genres.insert(i18n("Gothic Rock"), "Gothic Rock");
  genres.insert(i18n("Gothic"), "Gothic");
  genres.insert(i18n("Grunge"), "Grunge");
  genres.insert(i18n("Hardcore"), "Hardcore");
  genres.insert(i18n("Hard Rock"), "Hard Rock");
  genres.insert(i18n("Heavy Metal"), "Heavy Metal");
  genres.insert(i18n("Hip-Hop"), "Hip-Hop");
  genres.insert(i18n("House"), "House");
  genres.insert(i18n("Humor"), "Humor");
  genres.insert(i18n("Indie"), "Indie");
  genres.insert(i18n("Industrial"), "Industrial");
  genres.insert(i18n("Instrumental Pop"), "Instrumental Pop");
  genres.insert(i18n("Instrumental Rock"), "Instrumental Rock");
  genres.insert(i18n("Instrumental"), "Instrumental");
  genres.insert(i18n("Jazz+Funk"), "Jazz+Funk");
  genres.insert(i18n("Jazz"), "Jazz");
  genres.insert(i18n("JPop"), "JPop");
  genres.insert(i18n("Jungle"), "Jungle");
  genres.insert(i18n("Latin"), "Latin");
  genres.insert(i18n("Lo-Fi"), "Lo-Fi");
  genres.insert(i18n("Meditative"), "Meditative");
  genres.insert(i18n("Merengue"), "Merengue");
  genres.insert(i18n("Metal"), "Metal");
  genres.insert(i18n("Musical"), "Musical");
  genres.insert(i18n("National Folk"), "National Folk");
  genres.insert(i18n("Native American"), "Native American");
  genres.insert(i18n("Negerpunk"), "Negerpunk");
  genres.insert(i18n("New Age"), "New Age");
  genres.insert(i18n("New Wave"), "New Wave");
  genres.insert(i18n("Noise"), "Noise");
  genres.insert(i18n("Oldies"), "Oldies");
  genres.insert(i18n("Opera"), "Opera");
  genres.insert(i18n("Other"), "Other");
  genres.insert(i18n("Polka"), "Polka");
  genres.insert(i18n("Polsk Punk"), "Polsk Punk");
  genres.insert(i18n("Pop-Funk"), "Pop-Funk");
  genres.insert(i18n("Pop/Funk"), "Pop/Funk");
  genres.insert(i18n("Pop"), "Pop");
  genres.insert(i18n("Porn Groove"), "Porn Groove");
  genres.insert(i18n("Power Ballad"), "Power Ballad");
  genres.insert(i18n("Pranks"), "Pranks");
  genres.insert(i18n("Primus"), "Primus");
  genres.insert(i18n("Progressive Rock"), "Progressive Rock");
  genres.insert(i18n("Psychedelic Rock"), "Psychedelic Rock");
  genres.insert(i18n("Psychedelic"), "Psychedelic");
  genres.insert(i18n("Punk Rock"), "Punk Rock");
  genres.insert(i18n("Punk"), "Punk");
  genres.insert(i18n("R&B"), "R&B");
  genres.insert(i18n("Rap"), "Rap");
  genres.insert(i18n("Rave"), "Rave");
  genres.insert(i18n("Reggae"), "Reggae");
  genres.insert(i18n("Retro"), "Retro");
  genres.insert(i18n("Revival"), "Revival");
  genres.insert(i18n("Rhythmic Soul"), "Rhythmic Soul");
  genres.insert(i18n("Rock & Roll"), "Rock & Roll");
  genres.insert(i18n("Rock"), "Rock");
  genres.insert(i18n("Salsa"), "Salsa");
  genres.insert(i18n("Samba"), "Samba");
  genres.insert(i18n("Satire"), "Satire");
  genres.insert(i18n("Showtunes"), "Showtunes");
  genres.insert(i18n("Ska"), "Ska");
  genres.insert(i18n("Slow Jam"), "Slow Jam");
  genres.insert(i18n("Slow Rock"), "Slow Rock");
  genres.insert(i18n("Sonata"), "Sonata");
  genres.insert(i18n("Soul"), "Soul");
  genres.insert(i18n("Sound Clip"), "Sound Clip");
  genres.insert(i18n("Soundtrack"), "Soundtrack");
  genres.insert(i18n("Southern Rock"), "Southern Rock");
  genres.insert(i18n("Space"), "Space");
  genres.insert(i18n("Speech"), "Speech");
  genres.insert(i18n("Swing"), "Swing");
  genres.insert(i18n("Symphonic Rock"), "Symphonic Rock");
  genres.insert(i18n("Symphony"), "Symphony");
  genres.insert(i18n("Synthpop"), "Synthpop");
  genres.insert(i18n("Tango"), "Tango");
  genres.insert(i18n("Techno-Industrial"), "Techno-Industrial");
  genres.insert(i18n("Techno"), "Techno");
  genres.insert(i18n("Terror"), "Terror");
  genres.insert(i18n("Thrash Metal"), "Thrash Metal");
  genres.insert(i18n("Top 40"), "Top 40");
  genres.insert(i18n("Trailer"), "Trailer");
  genres.insert(i18n("Trance"), "Trance");
  genres.insert(i18n("Tribal"), "Tribal");
  genres.insert(i18n("Trip-Hop"), "Trip-Hop");
  genres.insert(i18n("Vocal"), "Vocal");

  genre->insertStringList(genres.keys());
  // Specify to only accept wav files
  file->setFilter("*.wav|Wav Files");
}

/**
 * When the user presses the encode button create a job with all of the current
 * selection options and emit a signal with it.
 */
void EncodeFileImp::encode(){
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

  newJob->track_title = track_title->text();
  newJob->track_artist = track_artist->text();
  newJob->track_comment = track_comment->text();

  emit(startJob(newJob));

  // Same message and *strings* from tracksimp.cpp
  int counter(1);
  KMessageBox::information(this,
  i18n("%1 Job(s) have been started.  You can watch their progress in the " \
	  "jobs section.").arg(counter),
 i18n("Jobs have started"), i18n("Jobs have started"));
}


#include "encodefileimp.moc"
