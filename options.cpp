#include "options.h"

#include "cdconfig.h"
#include "ripconfig.h"
#include "encoderconfig.h"
#include "general.h"

#include <kautoconfig.h>
#include <kdialogbase.h>
#include <klocale.h>
#include <kiconloader.h>
#include <qvbox.h>

#include <qpushbutton.h>
#include <kmessagebox.h>
#include <kprocess.h>

#include "wizard.h"
#include <kconfig.h>
#include <qcombobox.h>
#include <qlineedit.h>

#include <kcmoduleloader.h>
#include <kcmoduleinfo.h>

#define ENCODER_EXE_STRING "encoderExe_"
#define ENCODER_ARGS_STRING "encoderCommandLine_"
#define ENCODER_EXTENSION_STRING "encoderExtension_"
#define ENCODER_PERCENTLENGTH_STRING "encoderPercentLength_"

/**
 * Constructor
 */ 
Options::Options(QObject* parent, const char* name) : QObject(parent, name), oldEncoderSelection(-1){

  kautoconfig = new KAutoConfig(this, "Settings");
  options = new KDialogBase(KDialogBase::IconList, i18n("Settings"), KDialogBase::Default | KDialogBase::Ok | KDialogBase::Apply | KDialogBase::Cancel ,KDialogBase::Ok, 0, "Settings Dialog", true, true);
  connect(options, SIGNAL(okClicked()), this, SLOT(okClicked()));
  connect(options, SIGNAL(applyClicked()), this, SLOT(applyClicked()));
  connect(options, SIGNAL(defaultClicked()), this, SLOT(defaultClicked()));
  
  QVBox *frame = options->addVBoxPage(i18n("General"),i18n("General"), SmallIcon("package_settings", 32));
  General *general = new General(frame, "general");
  kautoconfig->addWidget(general, "general");

  frame = options->addVBoxPage(i18n("CD Config"),i18n("CD Config"), SmallIcon("network", 32));
  cdConfig = new CdConfig(frame, "CdConfig");
  connect(cdConfig->configureAudioCDButton, SIGNAL(clicked()), this, SLOT(configureAudioCD()));
  kautoconfig->addWidget(cdConfig, "cdconfig");

  frame = options->addVBoxPage(i18n("Ripper Config"),i18n("Ripper Config"), SmallIcon("shredder", 32));
  ripConfig = new RipConfig(frame, "RipConfig");
  kautoconfig->addWidget(ripConfig, "ripconfig");

  frame = options->addVBoxPage(i18n("Encoder Config"),i18n("Encoder Config"), SmallIcon("filter", 32));
  encoderConfig = new EncoderConfig(frame, "EncoderConfig");
  kautoconfig->addWidget(encoderConfig, "encoderconfig");
  kautoconfig->ignoreSubWidget((QWidget*)(encoderConfig->EncoderConfigGroupBox));
  
  // Because WE don't segfault on our users...
  if(KService::serviceByDesktopPath("Settings/Sound/cddb.desktop") != 0){
    frame = options->addVBoxPage(i18n("CDDB Config"),i18n("CDDB Config"), SmallIcon("cdaudio_unmount", 32));
    KCModuleInfo info("Settings/Sound/cddb.desktop", "settings");
    KCModule *m = KCModuleLoader::loadModule(info);
    m->reparent(((QWidget*)frame), 0, QPoint());
    m->load();
    connect(options, SIGNAL(okClicked()), m, SLOT(save()));
    connect(options, SIGNAL(applyClicked()), m, SLOT(save()));
    connect(options, SIGNAL(defaultClicked()), m, SLOT(defaults()));
  }

  connect(encoderConfig->playlistWizardButton, SIGNAL(clicked()), this, SLOT(playlistWizard()));
  connect(encoderConfig->encoderWizardButton, SIGNAL(clicked()), this, SLOT(encoderWizard()));

  connect(encoderConfig->encoder, SIGNAL(activated(int)), this, SLOT(loadEncoderConfig(int)));
  
  /** Encoder options **/
  KConfig &config = *KGlobal::config();
  config.setGroup("encoderconfig");
  int totalNumberOfEncoders = config.readNumEntry("numberOfEncoders",0);
  if( totalNumberOfEncoders == 0){
    encoderName.insert(0, i18n("OggEnc"));
    encoderConfig->encoder->insertItem(i18n("OggEnc"));
    encoderArgs.insert(0, "oggenc -o %o -a %artist -l %album -t %song -N %track %f");
    encoderExtension.insert(0, "ogg");
    encoderpercentLength.insert(0, 4);

    encoderName.insert(1, i18n("Lame"));
    encoderConfig->encoder->insertItem(i18n("Lame"));
    encoderArgs.insert(1, "lame --r3mix --tt %song --ta %artist --tl %album --ty %year --tn %track --tg %genre %f %o");
    encoderExtension.insert(1, "mp3");
    encoderpercentLength.insert(1, 2);

    encoderName.insert(2, i18n("Leave as Wav"));
    encoderConfig->encoder->insertItem(i18n("Leave as Wav"));
    encoderArgs.insert(2, "mv %f %o");
    encoderExtension.insert(2, "wav");
    encoderpercentLength.insert(2, 2);

    encoderName.insert(3, i18n("Other"));
    encoderConfig->encoder->insertItem(i18n("Other"));
    encoderArgs.insert(3, "");
    encoderExtension.insert(3, "");
    encoderpercentLength.insert(3, 2);
  }

  /***
   * The Encoders can be entirly loaded and are not hard coded.  You can add
   * remove them on the fly using the configure file, but a set of default
   * ones are include here.
   *
   * If you would like to add a default value to here contact
   * ben@meyerhome.net with apropriate values
   *
   * Encoder name - name of the encoder
   * Arguments - Command line string to encoder a file using that encoder
   * Extension - File extension that is generated.
   * Percent output length.  99.00% == 4, 99.9% == 3, 99% == 2
   */
  for(int i=0; i < totalNumberOfEncoders; i++){
    encoderName.insert(i, config.readEntry(QString(ENCODER_EXE_STRING "%1").arg(i),""));
    encoderConfig->encoder->insertItem(config.readEntry(QString(ENCODER_EXE_STRING "%1").arg(i),""),i);
    encoderArgs.insert(i, config.readEntry(QString(ENCODER_ARGS_STRING "%1").arg(i),""));
    encoderExtension.insert(i, config.readEntry(QString(ENCODER_EXTENSION_STRING "%1").arg(i),""));
    encoderpercentLength.insert(i, config.readNumEntry(QString(ENCODER_PERCENTLENGTH_STRING "%1").arg(i),2));
  }

  // Set the current item and settings.
  int currentItem = config.readNumEntry("encoderCurrentItem",0);
  encoderConfig->encoder->setCurrentItem(currentItem);
  loadEncoderConfig(encoderConfig->encoder->currentItem());
  /* All done with the encoder */
  
  kautoconfig->retrieveSettings();
  options->show();
}

/**
 * Deconstructor
 */ 
Options::~Options(){
  delete kautoconfig;

  KConfig &config = *KGlobal::config();
  config.setGroup("encoderconfig");
  config.writeEntry("encoderCurrentItem", encoderConfig->encoder->currentItem());
  
  QMap<int, QString>::Iterator it;
  for( it = encoderName.begin(); it != encoderName.end(); ++it )
    config.writeEntry(QString(ENCODER_EXE_STRING "%1").arg(it.key()), it.data());
  for( it = encoderArgs.begin(); it != encoderArgs.end(); ++it )
    config.writeEntry(QString(ENCODER_ARGS_STRING "%1").arg(it.key()), it.data());
  for( it = encoderExtension.begin(); it != encoderExtension.end(); ++it )
    config.writeEntry(QString(ENCODER_EXTENSION_STRING "%1").arg(it.key()), it.data());
  QMap<int, int>::Iterator nit;
  for( nit = encoderpercentLength.begin(); nit != encoderpercentLength.end(); ++nit )
    config.writeEntry(QString(ENCODER_PERCENTLENGTH_STRING "%1").arg(nit.key()), nit.data());

  config.writeEntry("encoderCurrentItem", encoderConfig->encoder->currentItem());
  config.writeEntry("numberOfEncoders", encoderConfig->encoder->count());
}

void Options::okClicked(){
  if(kautoconfig->saveSettings())
   emit( readNewOptions() );
  emit( closeOptions() );
}

void Options::applyClicked(){
  if( kautoconfig->saveSettings() )
    emit( readNewOptions() );
}

void Options::defaultClicked(){
  kautoconfig->resetSettings();
}

/*****  ****/

/**
 * open the AudioCD ioslave configure widget.
 */ 
void Options::configureAudioCD(){
  KMessageBox::information(options,
    i18n("The CDDA tab is the only one that is utilized at this time."),
    i18n("CDDA Tab"), i18n("CDDA Tab"));
  KShellProcess proc;
  proc << "kcmshell" << "audiocd";
  proc.start(KShellProcess::DontCare,  KShellProcess::NoCommunication);
}

/**
 * Load up the wizard with the playlist string.  Save it if OK is hit.
 */
void Options::playlistWizard(){
  fileWizard wizard(options, "Playlist File FormatWizard", true);
  wizard.playlistFormat->setText(encoderConfig->playlistFileFormat->text());

  // Show dialog and save results if ok is pressed.
  bool okClicked = wizard.exec();
  if(okClicked){
    encoderConfig->playlistFileFormat->setText(wizard.playlistFormat->text());
  }
}

/**
 * Load up the wizard with the encoder playlist string.  Save it if OK is hit.
 */
void Options::encoderWizard(){
  fileWizard wizard(options, "Encoder File Format Wizard", true);
  wizard.playlistFormat->setText(encoderConfig->fileFormat->text());

  // Show dialog and save results if ok is pressed.
  bool okClicked = wizard.exec();
  if(okClicked){
    encoderConfig->fileFormat->setText(wizard.playlistFormat->text());
  }
}

/**
 * Load the settings for this encoder.
 * @param index the selected item in the drop down menu.
 */
void Options::loadEncoderConfig(int index){
  if(encoderArgs[oldEncoderSelection] != encoderConfig->encoderCommandLine->text() && oldEncoderSelection != -1){
    encoderArgs.insert(oldEncoderSelection, encoderConfig->encoderCommandLine->text());
  }
  if(encoderExtension[oldEncoderSelection] != encoderConfig->encoderExtensionLineEdit->text() && oldEncoderSelection != -1){
    encoderExtension.insert(oldEncoderSelection, encoderConfig->encoderExtensionLineEdit->text());
  }
  oldEncoderSelection = index;

  // Now you can load the new settings.
  encoderConfig->encoderCommandLine->setText(encoderArgs[index]);
  encoderConfig->encoderExtensionLineEdit->setText(encoderExtension[index]);
}

