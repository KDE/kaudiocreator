/***************************************************************************
                                kaudiocreator.h
                             -------------------
    begin                : Friday Feb 9 2002
    copyright            : (C) 2001 by Benjamin Meyer
    email                : ben-dev@meyerhome.net
 ***************************************************************************/

#ifndef KAUDIOCREATOR_H
#define KAUDIOCREATOR_H

#include <kmainwindow.h>

class KJanusWidget;
class TracksConfigImp;
class CdConfigImp;
class RipConfigImp;
class EncoderConfigImp;
class QueConfigImp;

class KAudioCreator : public KMainWindow {

Q_OBJECT

signals:

public:
  KAudioCreator( QWidget* parent = 0, const char* name = 0);
  ~KAudioCreator();

private slots:
  void viewTracks();
  void viewCdConfig();
  void viewRipConfig();
  void viewEncoderConfig();
  void viewQue();
  void configuretoolbars();

private:
  KJanusWidget *janusWidget;
  TracksConfigImp *tracksConfig;
  int trackPage;
  CdConfigImp *cdConfig;
  int cdPage;
  RipConfigImp *ripConfig;
  int ripPage;
  EncoderConfigImp *encoderConfig;
  int encoderPage;
  QueConfigImp *queConfig;
  int quePage;

};

#endif

// kmp3creator.h

