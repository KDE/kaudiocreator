/***************************************************************************
                             encoderconfigimp.h
                             -------------------
    begin                : Friday Feb 9 2002
    copyright            : (C) 2001 by Benjamin Meyer
    email                : ben-dev@meyerhome.net
 ***************************************************************************/

#ifndef ENCODERCONFIGIMP_H
#define ENCODERCONFIGIMP_H

#include "encoderconfig.h"
#include <qmap.h>

/**
 * Class mainly build to manage encoder list.
 */ 
class EncoderConfigImp : public EncoderConfig {

Q_OBJECT

public:
  EncoderConfigImp( QWidget* parent = 0, const char* name = 0);

private slots:
  void addEncoderSlot();
  void removeEncoderSlot();
  void configureEncoderSlot();
  void setCurrentEncoderSlot();

  void updateEncoder(QObject * obj);
  void updateEncoder(const char *dialogName);
  void loadEncoderList();
  
private:
  // Name, groupName
  QMap<QString, QString> encoderNames;
  
};

#endif

