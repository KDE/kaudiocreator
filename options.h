#ifndef OPTIONS_H
#define OPTIONS_H

#include <kdialogbase.h>
#include <qobject.h>
#include <qmap.h>

class KDialogBase;
class CdConfig;
class RipConfig;
class EncoderConfig;
class KAutoConfig;
class General;

class Options : public QObject {

Q_OBJECT

signals: 
  void readNewOptions();
  void closeOptions();

public: 
  Options(QObject* parent = 0, const char* name = 0);
  ~Options();

private slots:
  void okClicked();
  void applyClicked();
  void defaultClicked();
  
  void configureAudioCD();

  void playlistWizard();
  void encoderWizard();
  void loadEncoderConfig(int index);

  void updateExample();
  
private:
  General* general;
  KDialogBase* options;
  CdConfig *cdConfig;
  RipConfig *ripConfig;
  EncoderConfig *encoderConfig;

  KAutoConfig *kautoconfig;

  QMap<int, QString> encoderName;
  QMap<int, QString> encoderArgs;
  QMap<int, QString> encoderExtension;
  QMap<int, int> encoderpercentLength;
  int oldEncoderSelection;

}; 

#endif

