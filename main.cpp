/**
 * Copyright (C) 2003-2005 Benjamin C Meyer (ben at meyerhome dot net)
 * Copyright (C) 2016 Leslie Zhai <xiang.zhai@i-soft.com.cn>
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

#include <QApplication>
#include <QCommandLineParser>

#include "kaudiocreator.h"
#include <kcmdlineargs.h>
#include <kaboutdata.h>
#include <klocalizedstring.h>

int main(int argc, char *argv[]) {
  QApplication a(argc, argv);

  KLocalizedString::setApplicationDomain("kaudiocreator");
  KAboutData aboutData("kaudiocreator", i18n("KAudioCreator"), "1.3");
  aboutData.setLicense(KAboutLicense::GPL);
  aboutData.addAuthor(i18n("Gerd Fleischer"), i18n("Maintainer"), "gerdfleischer@web.de");
  aboutData.addAuthor(i18n("Benjamin Meyer"), i18n("Original author"), "ben+kaudiocreator@meyerhome.net");
  aboutData.setOrganizationDomain(QByteArray("kde.org"));
  aboutData.setDesktopFileName(QStringLiteral("org.kde.kaudiocreator"));
  KAboutData::setApplicationData(aboutData);

  a.setWindowIcon(QIcon::fromTheme(QStringLiteral("kaudiocreator")));

  // command line
  QCommandLineParser parser;
  QCommandLineOption option("+[device]", i18n("CD device path"));
  parser.addOption(option);
  parser.addVersionOption();
  parser.addHelpOption();
  aboutData.setupCommandLine(&parser);
  parser.process(a);
  aboutData.processCommandLine(&parser);
  KAudioCreator *app = new KAudioCreator();

  const QStringList args = parser.positionalArguments();
  if (args.count() > 0) app->setDevice(args.at(0));
  app->show();
  return a.exec();
}

