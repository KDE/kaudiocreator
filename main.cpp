/***************************************************************************
                                  main.cpp
                             -------------------
    begin                : Sun Feb 13 01:05:24 EST 2000
    copyright            : (C) 2000 by Benjamin Meyer
    email                : ben-dev@meyerhome.net
 ***************************************************************************/

#include <kapplication.h>
#include "kaudiocreator.h"

//#include <string.h>
#include <kcmdlineargs.h>

#include <kaboutdata.h>
#include <klocale.h>
//#include <stdlib.h>
//#include <stdio.h>

int main(int argc, char *argv[]){

  // about data
  KAboutData aboutData("kaudiocreator", I18N_NOOP("KAudioCreator"), "0.1",
    I18N_NOOP("CD ripper and audio encoder frontend"), KAboutData::License_LGPL, "(c) 2002, Benjamin Meyer",
    0, "http://www.csh.rit.edu/~benjamin/");
    aboutData.addAuthor("Benjamin Meyer", I18N_NOOP("Original author"), "ben@meyerhome.net");

  // command line
  KCmdLineArgs::init(argc, argv, &aboutData);
  KApplication a(argc, argv);
  KAudioCreator *app = new KAudioCreator(0, "MainWindow");
  a.setMainWidget(app);
  app->show();
  return a.exec();
}

// main.cpp

