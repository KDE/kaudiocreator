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

#include <kapplication.h>

#include "kaudiocreator.h"
#include <kcmdlineargs.h>
#include <kaboutdata.h>

int main(int argc, char *argv[]){
  KAboutData aboutData("kaudiocreator", I18N_NOOP("KAudioCreator"), "1.01",
    I18N_NOOP("CD ripper and audio encoder frontend"), KAboutData::License_LGPL, "(c) 2003, Benjamin Meyer",
    0, "http://www.csh.rit.edu/~benjamin/");
    aboutData.addAuthor("Benjamin Meyer", I18N_NOOP("Original author"), "ben+kaudiocreator@meyerhome.net", "http://www.csh.rit.edu/~benjamin/");

  // command line
  KCmdLineArgs::init(argc, argv, &aboutData);
  KApplication a(argc, argv);
  KAudioCreator *app = new KAudioCreator(0, "MainWindow");
  a.setMainWidget(app);
  app->show();
  return a.exec();
}

