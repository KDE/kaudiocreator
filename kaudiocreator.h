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
class TracksImp;
class JobQueImp;
class Ripper;
class Encoder;

class KAudioCreator : public KMainWindow {

Q_OBJECT

signals:

public:
  KAudioCreator( QWidget* parent = 0, const char* name = 0);

protected:
  virtual bool queryClose();

private slots:
  void configuretoolbars();
  void saveToolbarConfig();
  void showSettings();

private:
  KJanusWidget *janusWidget;
  TracksImp *tracks;
  JobQueImp *jobQue;
  Ripper *ripper;
  Encoder *encoder;

};

#endif

