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
class QCloseEvent;
class KDialogBase;

class KAudioCreator : public KMainWindow {

Q_OBJECT

signals:

public:
  KAudioCreator( QWidget* parent = 0, const char* name = 0);

protected:
  virtual bool queryClose();

private slots:
  void viewTracks();
  void viewQue();
  void configuretoolbars();
  void newToolbarConfig();
  void quit();
  void showOptions();

private:
  KJanusWidget *janusWidget;
  KDialogBase* options;
  TracksConfigImp *tracksConfig;
  int trackPage;
  CdConfigImp *cdConfig;
  RipConfigImp *ripConfig;
  EncoderConfigImp *encoderConfig;
  QueConfigImp *queConfig;
  int quePage;

};

#endif

// kaudiocreator.h

