/**
 * Copyright (C) 2003-2005 Benjamin C Meyer (ben at meyerhome dot net)
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

#include <kdebug.h>

#include "encoderconfigimp.h"
#include "prefs.h"
#include "wizard.h"

#include <QPushButton>

#include <kstandarddirs.h>
#include <kconfigdialog.h>
#include <kmessagebox.h>
#include <klocale.h>
#include <kglobal.h>
#include <kconfig.h>

/**
 * Constructor.
 */
EncoderConfigImp::EncoderConfigImp( QWidget* parent) :
  QWidget(parent)
{
  setupUi(this);
  connect(addEncoder, SIGNAL(clicked()), this, SLOT(addEncoderSlot()));
  connect(removeEncoder, SIGNAL(clicked()), this, SLOT(removeEncoderSlot()));
  connect(configureEncoder, SIGNAL(clicked()), this, SLOT(configureEncoderSlot()));
  connect(kcfg_currentEncoder, SIGNAL(itemDoubleClicked(QListWidgetItem *)),this, SLOT(configureEncoderSlot()));
  connect(encoderWizardButton, SIGNAL(clicked()), this, SLOT(encoderWizard()));

  if (KStandardDirs::findExe("nice") == QString())
    niceLevelBox->setEnabled(false);

  // If there are no encoders then store the three default ones.
  if (Prefs::lastKnownEncoder() == 0) {
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
    encPrefs->setEncoderName(i18n("WAV"));
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
    Prefs::self()->writeConfig();
  }

  loadEncoderList();
}

/**
 * Clear map
 * Clear listbox
 * Load list of encoders.
 */ 
void EncoderConfigImp::loadEncoderList() {
    encoderNames.clear();
    kcfg_currentEncoder->clear();

    bool foundCurrentEncoder = false;

    int lastEncoder = 0;
    int lastKnownEncoder = Prefs::lastKnownEncoder();
    ++lastKnownEncoder;
    for (int i=0; i<=lastKnownEncoder; ++i) {
        QString currentGroup = QString("Encoder_%1").arg(i);
        if (EncoderPrefs::hasPrefs(currentGroup)) {
            lastEncoder = i;
            EncoderPrefs *encPrefs = EncoderPrefs::prefs(currentGroup);
            QString encoderName = encPrefs->encoderName();
            kcfg_currentEncoder->addItem(new QListWidgetItem(encoderName));
            encoderNames.insert(encoderName, currentGroup);
            if (Prefs::currentEncoder() == i) {
                foundCurrentEncoder = true;
            }
        }
    }

    if (lastEncoder != Prefs::lastKnownEncoder()) {
        Prefs::setLastKnownEncoder(lastEncoder);
        Prefs::self()->writeConfig();
    }

    // Make sure that the current encoder is valid.
    if (!foundCurrentEncoder && kcfg_currentEncoder->count() > 0) {
        kcfg_currentEncoder->setCurrentItem(0);
     }
}

/**
 * Find empty group
 * bring up dialog for that group.
 */
void EncoderConfigImp::addEncoderSlot() {
    bool foundEmptyGroup = false;
    uint number = 0;
    QString groupName;
    while (!foundEmptyGroup) {
        groupName = QString("Encoder_%1").arg(number);
        if (!EncoderPrefs::hasPrefs(groupName))
            foundEmptyGroup = true;
        else
            number++;
    }

  if(KConfigDialog::showDialog(groupName.toLatin1()))
    return;

  KConfigDialog *dialog = new KConfigDialog(this, groupName.toLatin1(), EncoderPrefs::prefs(groupName));
  dialog->setFaceType(KPageDialog::Plain);
  dialog->setButtons(KDialog::Ok | KDialog::Cancel | KDialog::Help);
  dialog->setCaption(i18n("Configure Encoder"));
  dialog->addPage(new EncoderEdit(0/*, groupName.toLatin1()*/), i18n("Encoder Configuration"), "package_settings");
  connect(dialog, SIGNAL(settingsChanged(const QString &)), this, SLOT(loadEncoderList()));
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
  if(!kcfg_currentEncoder->currentItem()){
    KMessageBox:: sorry(this, i18n("Please select an encoder."), i18n("No Encoder Selected"));
    return;
  }	
  if(kcfg_currentEncoder->count() <= 1){
    KMessageBox:: sorry(this, i18n("At least one encoder must exist."), i18n("Can Not Remove"));
    return;
  }
  if(KMessageBox::warningContinueCancel(this, i18n("Delete encoder?"), i18n("Delete Encoder"),KStandardGuiItem::del())
      == KMessageBox::Cancel )
    return;

  QString groupName = encoderNames[kcfg_currentEncoder->currentItem()->text()];
  kcfg_currentEncoder->removeItemWidget(kcfg_currentEncoder->currentItem());

  delete KConfigDialog::exists(groupName.toLatin1());

  EncoderPrefs::deletePrefs(groupName);
  loadEncoderList();
}

/**
 * If
 * Something is selected
 * Group exists
 * Then
 * Bring up dialog
 */ 
void EncoderConfigImp::configureEncoderSlot() {
  if(!kcfg_currentEncoder->currentItem()){
    KMessageBox:: sorry(this, i18n("Please select an encoder."), i18n("No Encoder Selected"));
    return;
  }
  QString groupName = encoderNames[kcfg_currentEncoder->currentItem()->text()];
  KConfig &config = *KGlobal::config();
  if(!config.hasGroup(groupName))
    return;

  if(KConfigDialog::showDialog(groupName.toLatin1()))
    return;

  KConfigDialog *dialog = new KConfigDialog(this, groupName.toLatin1(), EncoderPrefs::prefs(groupName));
  dialog->setFaceType(KPageDialog::Plain);
  dialog->setButtons(KDialog::Ok | KDialog::Cancel | KDialog::Help);
  dialog->setCaption(i18n("Configure Encoder"));
  dialog->addPage(new EncoderEdit(0/*, groupName.toLatin1()*/), i18n("Encoder Configuration"), "package_settings");
  connect(dialog, SIGNAL(settingsChanged(const QString &)), this, SIGNAL(encoderUpdated()));
  connect(dialog, SIGNAL(settingsChanged(const QString &)), this, SLOT(updateEncoder(const QString &)));
  dialog->show();
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
void EncoderConfigImp::updateEncoder(const QString &dialogName){
  QString groupName = dialogName;
  QString encoderName;
  bool found = false;
  QMap<QString, QString>::Iterator it;
  for ( it = encoderNames.begin(); it != encoderNames.end(); ++it ) {
    if(it.value() == groupName){
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

  QList<QListWidgetItem *> items = kcfg_currentEncoder->findItems(encoderName, Qt::MatchExactly);
  if(items.isEmpty())
    return;
  items.at(0)->setText(newName);

  encoderNames.insert(newName, groupName);
  encoderNames.remove(encoderName);
}

/**
 * Load up the wizard with the encoder fileFormat string.  Save it if OK is hit.
 */
void EncoderConfigImp::encoderWizard(){
  fileWizard wizard(this);
  wizard.fileFormat->setText(kcfg_fileFormat->text());

  // Show dialog and save results if ok is pressed.
  bool okClicked = wizard.exec();
  if(okClicked){
    kcfg_fileFormat->setText(wizard.fileFormat->text());
  }
}

QHash<QString, EncoderPrefs *> *EncoderPrefs::m_prefs = 0;

EncoderPrefs *EncoderPrefs::prefs(const QString &groupName)
{
  if (!m_prefs)
  {
     m_prefs = new QHash<QString, EncoderPrefs *>;
//     m_prefs->setAutoDelete(true);
  }
   QHash<QString, EncoderPrefs *>::iterator encPrefsIt = m_prefs->find(groupName);
  if (encPrefsIt != m_prefs->end())
    return encPrefsIt.value();
    
  EncoderPrefs *encPrefs = new EncoderPrefs(groupName);
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

