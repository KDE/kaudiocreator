/**
 * This file is part of the KAudioCreator package
 * Copyright (C) 2003 Benjamin C Meyer (ben+kaudiocreator at meyerhome dot net)
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
 * the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include "encoderconfigimp.h"
#include "encoderedit.h"
#include "prefs.h"

#include <qpushbutton.h>
#include <qlineedit.h>
#include <kconfigdialog.h>
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
  connect(kcfg_currentEncoder, SIGNAL(doubleClicked ( QListBoxItem * )),this, SLOT(configureEncoderSlot()));

  // If there are no encoders then store the three default ones.
  if( Prefs::lastKnownEncoder() == 0){
    EncoderPrefs *encPrefs;

    encPrefs = EncoderPrefs::prefs("Encoder_0");
    encPrefs->setEncoderName(i18n("Ogg Vorbis"));
    encPrefs->setCommandLine("oggenc -o %o --artist %{artist} --album %{albumtitle} --title %{title} --date %{year} --tracknum %{number} --genre %{genre} %f");
    encPrefs->setExtension("ogg");
    encPrefs->setPercentLength(4);
    encPrefs->writeConfig();

    encPrefs = EncoderPrefs::prefs("Encoder_1");
    encPrefs->setEncoderName(i18n("MP3"));
    encPrefs->setCommandLine("lame --preset standard --tt %{title} --ta %{artist} --tl %{albumtitle} --ty %{year} --tn %{number} --tg %{genre} %f %o");
    encPrefs->setExtension("mp3");
    encPrefs->setPercentLength(2);
    encPrefs->writeConfig();

    encPrefs = EncoderPrefs::prefs("Encoder_2");
    encPrefs->setEncoderName(i18n("Wav"));
    encPrefs->setCommandLine("mv %f %o");
    encPrefs->setExtension("wav");
    encPrefs->setPercentLength(2);
    encPrefs->writeConfig();

    encPrefs = EncoderPrefs::prefs("Encoder_3");
    encPrefs->setEncoderName(i18n("FLAC"));
    encPrefs->setCommandLine("flac --best -o %o --tag=Artist=%{artist} --tag=Album=%{albumtitle} --tag=Date=%{year} --tag=Title=%{title} --tag=Tracknumber=%{number} --tag=Genre=%{genre} %f");
    encPrefs->setExtension("flac");
    encPrefs->setPercentLength(2);
    encPrefs->writeConfig();

    Prefs::setLastKnownEncoder(3);
    Prefs::writeConfig();
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
  kcfg_currentEncoder->clear();

  bool foundCurrentEncoder = false;

  int lastEncoder = 0;
  int lastKnownEncoder = Prefs::lastKnownEncoder();
  lastKnownEncoder++;
  for( int i=0; i<=lastKnownEncoder; i++ ){
    QString currentGroup = QString("Encoder_%1").arg(i);
    if(EncoderPrefs::hasPrefs(currentGroup)){
      lastEncoder = i;
      EncoderPrefs *encPrefs = EncoderPrefs::prefs(currentGroup);
      QString encoderName = encPrefs->encoderName();
      kcfg_currentEncoder->insertItem(encoderName);
      encoderNames.insert(encoderName, currentGroup);
      if(Prefs::currentEncoder() == i)
	foundCurrentEncoder = true;
    }
  }
  if(lastEncoder != Prefs::lastKnownEncoder()){
    Prefs::setLastKnownEncoder(lastEncoder);
    Prefs::writeConfig();
  }

  // Make sure that the current encoder is valid.
  if(!foundCurrentEncoder && kcfg_currentEncoder->count() > 0)
    kcfg_currentEncoder->setCurrentItem(0);
}

/**
 * Find empty group
 * bring up dialog for that group.
 */
void EncoderConfigImp::addEncoderSlot(){
  bool foundEmptyGroup = false;
  uint number = 0;
  QString groupName;
  while(!foundEmptyGroup){
    groupName = QString("Encoder_%1").arg(number);
    if(!EncoderPrefs::hasPrefs(groupName))
      foundEmptyGroup = true;
    else
      number++;
  }

  if(KConfigDialog::showDialog(groupName.latin1()))
    return;

  KConfigDialog *dialog = new KConfigDialog(this, groupName.latin1(), EncoderPrefs::prefs(groupName), 
                                            KDialogBase::Swallow,
                                            KDialogBase::Ok | KDialogBase::Cancel | KDialogBase::Help);
  dialog->setCaption(i18n("Configure Encoder"));
  dialog->addPage(new EncoderEdit(0, groupName.latin1()), i18n("Encoder Configuration"), "package_settings");
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
  if(!kcfg_currentEncoder->selectedItem()){
    KMessageBox:: sorry(this, i18n("Please select an encoder."), i18n("No Encoder Selected"));
    return;
  }
  if(kcfg_currentEncoder->count() <= 1){
    KMessageBox:: sorry(this, i18n("At least one encoder must exist."), i18n("Can Not Remove"));
    return;
  }
  if(KMessageBox::warningContinueCancel(this, i18n("Delete encoder?"), i18n("Delete Encoder"),KStdGuiItem::del())
      == KMessageBox::Cancel )
    return;

  QString groupName = encoderNames[kcfg_currentEncoder->currentText()];
  kcfg_currentEncoder->removeItem(kcfg_currentEncoder->currentItem());

  delete KConfigDialog::exists(groupName.latin1());

  EncoderPrefs::deletePrefs(groupName);
}

/**
 * If
 * Something is selected
 * Group exists
 * Then
 * Bring up dialog
 */
void EncoderConfigImp::configureEncoderSlot() {
  if(!kcfg_currentEncoder->selectedItem()){
    KMessageBox:: sorry(this, i18n("Please select an encoder."), i18n("No Encoder Selected"));
    return;
  }
  QString groupName = encoderNames[kcfg_currentEncoder->currentText()];
  KConfig &config = *KGlobal::config();
  if(!config.hasGroup(groupName))
    return;

  if(KConfigDialog::showDialog(groupName.latin1()))
    return;

  KConfigDialog *dialog = new KConfigDialog(this, groupName.latin1(), EncoderPrefs::prefs(groupName),
                                            KDialogBase::Swallow,
                                            KDialogBase::Ok | KDialogBase::Cancel | KDialogBase::Help);
  dialog->setCaption(i18n("Configure Encoder"));
  dialog->addPage(new EncoderEdit(0, groupName.latin1()), i18n("Encoder Configuration"), "package_settings");
  connect(dialog, SIGNAL(destroyed(QObject *)), this, SLOT(updateEncoder(QObject *)));
  connect(dialog, SIGNAL(settingsChanged()), this, SIGNAL(encoderUpdated()));
  connect(dialog, SIGNAL(settingsChanged(const char *)), this, SLOT(updateEncoder(const char *)));
  dialog->show();
}

/**
 * If object exists update encoder
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
  if(!EncoderPrefs::hasPrefs(groupName))
    return;
  QString newName = EncoderPrefs::prefs(groupName)->encoderName();
  if(newName == encoderName)
    return;

  QListBoxItem *item = kcfg_currentEncoder->findItem(encoderName);
  if(!item)
    return;
  kcfg_currentEncoder->changeItem(newName, kcfg_currentEncoder->index(item));

  encoderNames.insert(newName, groupName);
  encoderNames.erase(encoderName);
}

QDict<EncoderPrefs> *EncoderPrefs::m_prefs = 0;

EncoderPrefs *EncoderPrefs::prefs(const QString &groupName)
{
  if (!m_prefs)
  {
     m_prefs = new QDict<EncoderPrefs>();
     m_prefs->setAutoDelete(true);
  }
  EncoderPrefs *encPrefs = m_prefs->find(groupName);
  if (encPrefs)
    return encPrefs;

  encPrefs = new EncoderPrefs(groupName);
  encPrefs->readConfig();
  m_prefs->insert(groupName, encPrefs);
  return encPrefs;
}

bool EncoderPrefs::hasPrefs(const QString &groupName)
{
  KConfig &config = *KGlobal::config();
  return config.hasGroup(groupName);
}

void EncoderPrefs::deletePrefs(const QString &groupName)
{
  KConfig &config = *KGlobal::config();
  config.deleteGroup(groupName);
  if (!m_prefs)
    return;
  m_prefs->remove(groupName);
}

#include "encoderconfigimp.moc"

