/**
 * This file is part of the KAudioCreator package
 * Copyright (C) 2003-2005 Benjamin C Meyer (ben+kaudiocreator at meyerhome dot net)
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

#include <kapplication.h>

#include "kaudiocreator.h"
#include <kcmdlineargs.h>
#include <kaboutdata.h>

static const KCmdLineOptions options[] = {
  {"+[device]",I18N_NOOP( "CD device, can be a path or a media:/ URL" ),0},
  KCmdLineLastOption
};


int main(int argc, char *argv[]){
  KAboutData aboutData("kaudiocreator", I18N_NOOP("KAudioCreator"), "1.13",
    I18N_NOOP("CD ripper and audio encoder frontend"), KAboutData::License_LGPL, "(c) 2003-2005, Benjamin Meyer",
    0, "http://www.icefox.net/");
    aboutData.addAuthor("Benjamin Meyer", I18N_NOOP("Original author"), "ben+kaudiocreator@meyerhome.net", "http://www.icefox.net/");

  // command line
  KCmdLineArgs::init(argc, argv, &aboutData);
  KCmdLineArgs::addCmdLineOptions( options );
  KApplication a(argc, argv);
  KAudioCreator *app = new KAudioCreator(0, "MainWindow");

  // we need some strings from libkcddb for the cddb album dialog
  KGlobal::locale()->insertCatalogue("libkcddb");

  a.setMainWidget(app);

  KCmdLineArgs* args = KCmdLineArgs::parsedArgs();
  if ( args->count()>0 ) app->setDevice( args->arg( 0 ) );
  args->clear();
  app->show();
  return a.exec();
}

