#include "options.h"

#include "cdconfig.h"
#include "ripconfig.h"
#include "encoderconfigimp.h"
#include "general.h"

#include <kautoconfig.h>
#include <klocale.h>
#include <kiconloader.h>
#include <qvbox.h>

#include <qpushbutton.h>
#include <kmessagebox.h>
#include <kprocess.h>

#include "wizard.h"
#include <qlineedit.h>
#include <qlabel.h>

#include <kcmoduleloader.h>

/**
 * Constructor
 */ 
Options::Options(QObject* parent, const char* name) : QObject(parent, name), oldEncoderSelection(-1){

  kautoconfig = new KAutoConfig(this, "Settings");
  options = new KDialogBase(KDialogBase::IconList, i18n("Settings"), KDialogBase::Default | KDialogBase::Ok | KDialogBase::Apply | KDialogBase::Cancel ,KDialogBase::Ok, 0, "Settings Dialog", false, true);
  connect(options, SIGNAL(okClicked()), this, SLOT(okClicked()));
  connect(options, SIGNAL(applyClicked()), this, SLOT(applyClicked()));
  connect(options, SIGNAL(defaultClicked()), this, SLOT(defaultClicked()));
  
  QVBox *frame = options->addVBoxPage(i18n("General"),i18n("General"), SmallIcon("package_settings", 32));
  general = new General(frame, "general");
  kautoconfig->addWidget(general, "General");

  frame = options->addVBoxPage(i18n("CD"),i18n("CD Configuration"), SmallIcon("network", 32));
  cdConfig = new CdConfig(frame, "CdConfig");
  connect(cdConfig->configureAudioCDButton, SIGNAL(clicked()), this, SLOT(configureAudioCD()));
  kautoconfig->addWidget(cdConfig, "CD");
  
  // Because WE don't segfault on our users...
  if(KService::serviceByDesktopPath("Settings/Sound/cddb.desktop") != 0){
    frame = options->addVBoxPage(i18n("CDDB"),i18n("CDDB Configuration"), SmallIcon("cdaudio_unmount", 32));
    KCModuleInfo info("Settings/Sound/cddb.desktop", "settings");
    KCModule *m = KCModuleLoader::loadModule(info);
    m->reparent(((QWidget*)frame), 0, QPoint());
    m->load();
    connect(options, SIGNAL(okClicked()), m, SLOT(save()));
    connect(options, SIGNAL(applyClicked()), m, SLOT(save()));
    connect(options, SIGNAL(defaultClicked()), m, SLOT(defaults()));
  }
  
  frame = options->addVBoxPage(i18n("Ripper"),i18n("Ripper Configuration"), SmallIcon("shredder", 32));
  ripConfig = new RipConfig(frame, "Ripper");
  kautoconfig->addWidget(ripConfig, "Ripper");

  frame = options->addVBoxPage(i18n("Encoder"),i18n("Encoder Configuration"), SmallIcon("filter", 32));
  encoderConfigImp = new EncoderConfigImp(frame, "EncoderConfig");
  kautoconfig->addWidget(encoderConfigImp, "Encoder");
  
  connect(encoderConfigImp->playlistWizardButton, SIGNAL(clicked()), this, SLOT(playlistWizard()));
  connect(encoderConfigImp->encoderWizardButton, SIGNAL(clicked()), this, SLOT(encoderWizard()));

  kautoconfig->retrieveSettings();
  options->show();
}

/**
 * Deconstructor
 */ 
Options::~Options(){
  delete kautoconfig;
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
  wizard.playlistFormat->setText(encoderConfigImp->playlistFileFormat->text());

  // Show dialog and save results if ok is pressed.
  bool okClicked = wizard.exec();
  if(okClicked){
    encoderConfigImp->playlistFileFormat->setText(wizard.playlistFormat->text());
  }
}

/**
 * Load up the wizard with the encoder playlist string.  Save it if OK is hit.
 */
void Options::encoderWizard(){
  fileWizard wizard(options, "Encoder File Format Wizard", true);
  wizard.playlistFormat->setText(encoderConfigImp->fileFormat->text());

  // Show dialog and save results if ok is pressed.
  bool okClicked = wizard.exec();
  if(okClicked){
    encoderConfigImp->fileFormat->setText(wizard.playlistFormat->text());
  }
}

#include "options.moc"

