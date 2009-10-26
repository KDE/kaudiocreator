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

#include <kapplication.h>

#include "kaudiocreator.h"
#include <kcmdlineargs.h>
#include <kaboutdata.h>
#include <kglobal.h>
#include <klocale.h>


int main(int argc, char *argv[]){
  KAboutData aboutData("kaudiocreator", 0, ki18n("KAudioCreator"), "1.2.80",
    ki18n("CD ripper and audio encoder frontend"), KAboutData::License_LGPL,
          ki18n("(c) 2003-2005, Benjamin Meyer\n(c) 2008-2009, Gerd Fleischer"),
          KLocalizedString(), "http://kde-apps.org/content/show.php/KAudioCreator?content=107645");
    aboutData.addAuthor(ki18n("Gerd Fleischer"), ki18n("Maintainer"), "gerdfleischer@web.de", "http://www.gerdfleischer.de");
    aboutData.addAuthor(ki18n("Benjamin Meyer"), ki18n("Original author"), "ben+kaudiocreator@meyerhome.net", "http://www.icefox.net/");

  // command line
  KCmdLineArgs::init(argc, argv, &aboutData);

  KCmdLineOptions options;
  options.add("+[device]", ki18n( "CD device, can be a path or a media:/ URL" ));
  KCmdLineArgs::addCmdLineOptions( options );
  KApplication a(true);
  KAudioCreator *app = new KAudioCreator();

  // we need some strings from libkcddb for the cddb album dialog
  KGlobal::locale()->insertCatalog("libkcddb");

  KCmdLineArgs* args = KCmdLineArgs::parsedArgs();
  if ( args->count()>0 ) app->setDevice( args->arg( 0 ) );
  args->clear();
  app->show();
  return a.exec();
}

