/***************************************************************************
                                  main.cpp
                             -------------------
    begin                : Sun Feb 13 EST 2002
    copyright            : (C) 2002 by Benjamin Meyer
    email                : ben-dev@meyerhome.net
 ***************************************************************************/

#include <kapplication.h>

#include "kaudiocreator.h"
#include <kcmdlineargs.h>
#include <kaboutdata.h>

int main(int argc, char *argv[]){
  KAboutData aboutData("kaudiocreator", I18N_NOOP("KAudioCreator"), "0.90",
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

