#include "encoderconfigimp.h"
#include "encoderedit.h"

#include <qpushbutton.h>
#include <qlineedit.h>
#include <kautoconfigdialog.h>
#include <kmessagebox.h>
#include <klocale.h>
#include <kconfig.h>

/**
 * Constructor.
 */
EncoderConfigImp::EncoderConfigImp( QWidget* parent, const char* name) :
    EncoderConfig (parent, name) {
  connect(addEncoder, SIGNAL(clicked()), this, SLOT(addEncoderSlot()));
  connect(removeEncoder, SIGNAL(clicked()), this, SLOT(removeEncoderSlot()));
  connect(configureEncoder, SIGNAL(clicked()), this, SLOT(configureEncoderSlot()));
  connect(setCurrentEncoder, SIGNAL(clicked()), this, SLOT(setCurrentEncoderSlot()));

  KConfig &config = *KGlobal::config();
  config.setGroup("Encoder");
  int lastKnownEncoder = config.readNumEntry("lastKnownEncoder",0);

  // If there are no encoders then store the three default ones.
  if( lastKnownEncoder == 0){
    config.setGroup("Encoder_0");
    config.writeEntry("encoderName", i18n("OggEnc"));
    config.writeEntry("commandLine", "oggenc -o %o --artist %artist --album %album --title %song --tracknum %track --genre %genre %f");
    config.writeEntry("extension", "ogg");
    config.writeEntry("percentLength", 4);

    config.setGroup("Encoder_1");
    config.writeEntry("encoderName", i18n("Lame"));
    config.writeEntry("commandLine", "lame --r3mix --tt %song --ta %artist --tl %album --ty %year --tn %track --tg %genre %f %o");
    config.writeEntry("extension", "mp3");
    config.writeEntry("percentLength", 2);

    config.setGroup("Encoder_2");
    config.writeEntry("encoderName", i18n("Leave as a wav file"));
    config.writeEntry("commandLine", "mv %f %o");
    config.writeEntry("extension", "wav");
    config.writeEntry("percentLength", 2);

    config.setGroup("Encoder_3");
    config.writeEntry("encoderName", i18n("FLAC"));
    config.writeEntry("commandLine", "flac --best -o %o --tag=title=%artist --tag=album=%album --tag=title=%song --tag=tracknumber=%track -tag=genre=%genre %f");
    config.writeEntry("extension", "flac");
    config.writeEntry("percentLength", 2);

    config.setGroup("Encoder");
    config.writeEntry("lastKnownEncoder", 2);
  }
  
  loadEncoderList();
}

/**
 * Clear map
 * Clear listbox
 * Load list of encoders.
 */ 
void EncoderConfigImp::loadEncoderList(){
  encoderNames.clear();
  encoderList->clear();
  
  KConfig &config = *KGlobal::config();
  config.setGroup("Encoder");
  
  QString currentEncoderString = config.readEntry("currentEncoder");
  bool foundCurrentEncoder = false;
  
  uint lastEncoder = 0;
  uint lastKnownEncoder = config.readNumEntry("lastKnownEncoder",0);
  lastKnownEncoder++;
  for( uint i=0; i<=lastKnownEncoder; i++ ){
    QString currentGroup = QString("Encoder_%1").arg(i);
    if(config.hasGroup(currentGroup)){
      lastEncoder = i;
      config.setGroup(currentGroup);
      QString encoderName = config.readEntry("encoderName", i18n("Unknown Encoder"));
      encoderList->insertItem(encoderName);
      encoderNames.insert(encoderName, currentGroup);
      if(currentEncoderString == currentGroup)
	foundCurrentEncoder = true;    
    }
  }
  if((lastKnownEncoder-1) != lastEncoder){
    config.setGroup("Encoder");
    config.writeEntry("lastKnownEncoder", lastEncoder);
  }
  
  // Make sure that the current encoder is valid.
  if(foundCurrentEncoder){
    config.setGroup(currentEncoderString);
    currentEncoderName->setText(config.readEntry("encoderName"));
  }
}

/**
 * Find empty group
 * bring up dialog for that group.
 */ 
void EncoderConfigImp::addEncoderSlot(){
  KConfig &config = *KGlobal::config();
  bool foundEmptyGroup = false;
  uint number = 0;
  while(!foundEmptyGroup){
    if(!config.hasGroup(QString("Encoder_%1").arg(number)))
      foundEmptyGroup = true;
    else
      number++;
  }
 
  QString groupName = QString("Encoder_%1").arg(number);
  if(KAutoConfigDialog::showDialog(groupName.latin1()))
    return;
  KAutoConfigDialog *dialog = new KAutoConfigDialog(this, groupName.latin1(), KDialogBase::Swallow);
  dialog->addPage(new EncoderEdit(0, groupName.latin1()), i18n("Encoder Configuration"), groupName, "package_settings");
  connect(dialog, SIGNAL(settingsChanged()), this, SLOT(loadEncoderList()));
  dialog->show();
}

/**
 * If
 * Something is selected
 * There is more then 1 thing left
 * The user says ok to delete.
 * Is not the current encoder.
 * Then
 * The group is removed from the list
 * Deleted from the config.
 */ 
void EncoderConfigImp::removeEncoderSlot(){
  if(!encoderList->selectedItem()){
    KMessageBox:: sorry(this, i18n("Please select an encoder."), i18n("No Encoder Selected"));
    return;
  }	
  if(encoderList->count() <= 1){
    KMessageBox:: sorry(this, i18n("There must exist at least one encoder."), i18n("Can't remove."));
    return;
  }
  if(currentEncoderName->text() == encoderList->currentText()){
    KMessageBox:: sorry(this, i18n("Encoder is currently set as current.\nPlease select a different encoder as current before this one can be removed."), i18n("Can't remove."));
    return;
  }

  if(KMessageBox::questionYesNo(this, i18n("Delete Encoder?"), i18n("Delete Encoder"))
      == KMessageBox::No )
    return;
  
  QString groupName = encoderNames[encoderList->currentText()];
  KConfig &config = *KGlobal::config();
  config.deleteGroup(groupName);
  encoderList->removeItem(encoderList->currentItem());
}

/**
 * If
 * Something is selected
 * Group exists
 * Then
 * Bring up dialog
 */ 
void EncoderConfigImp::configureEncoderSlot() {
  if(!encoderList->selectedItem()){
    KMessageBox:: sorry(this, i18n("Please select an encoder."), i18n("No Encoder Selected"));
    return;
  }
  QString groupName = encoderNames[encoderList->currentText()];
  KConfig &config = *KGlobal::config();
  if(!config.hasGroup(groupName))
    return;

  if(KAutoConfigDialog::showDialog(groupName.latin1()))
    return;
    
  KAutoConfigDialog *dialog = new KAutoConfigDialog(this, groupName.latin1(), KDialogBase::Swallow);
  dialog->addPage(new EncoderEdit(0, groupName.latin1()), i18n("Encoder Configuration"), groupName, "package_settings");
  connect(dialog, SIGNAL(destroyed(QObject *)), this, SLOT(updateEncoder(QObject *)));
  connect(dialog, SIGNAL(settingsChanged()), this, SIGNAL(encoderUpdated()));
  connect(dialog, SIGNAL(settingsChanged(const char *)), this, SLOT(updateEncoder(const char *)));
  dialog->show();
}

void EncoderConfigImp::setCurrentEncoderSlot(){
  if(!encoderList->selectedItem()){
    KMessageBox:: sorry(this, i18n("Please select an encoder."), i18n("No Encoder Selected"));
    return;
  }
  currentEncoderName->setText(encoderList->currentText());

  KConfig &config = *KGlobal::config();
  config.setGroup("Encoder");
  config.writeEntry("currentEncoder", encoderNames[encoderList->currentText()]);
}

/**
 * If object exists update encoder.
 */ 
void EncoderConfigImp::updateEncoder(QObject * obj){
  if(!obj)
   return;
  updateEncoder(obj->name());
}

/**
 * If
 * Exists
 * Then
 * Get name
 * Make sure group exists
 * Update name
 * Update Map
 * If current encoder update also.
 */ 
void EncoderConfigImp::updateEncoder(const char *dialogName){
  QString groupName = dialogName;
  QString encoderName;
  bool found = false;
  QMap<QString, QString>::Iterator it;
  for ( it = encoderNames.begin(); it != encoderNames.end(); ++it ) {
    if(it.data() == groupName){
      found = true;
      encoderName = it.key();
    }
  }
  if(!found)
    return;
  KConfig &config = *KGlobal::config();
  if(!config.hasGroup(groupName))
    return;
  config.setGroup(groupName);
  QString newName = config.readEntry("encoderName");
  if(newName == encoderName)
    return;
  
  QListBoxItem *item = encoderList->findItem(encoderName);
  if(!item)
    return;
  encoderList->changeItem(newName, encoderList->index(item));

  if(currentEncoderName->text() == encoderName)
    currentEncoderName->setText(newName);
  encoderNames.insert(newName, groupName);
  encoderNames.erase(encoderName);
}


#include "encoderconfigimp.moc"

