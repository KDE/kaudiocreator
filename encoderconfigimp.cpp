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

#include <kstandarddirs.h>
#include <kconfigdialog.h>
#include <kmessagebox.h>
#include <klocale.h>
#include <kglobal.h>
#include <kconfig.h>
#include <krandom.h>

/**
 * Constructor.
 */
EncoderConfigImp::EncoderConfigImp(QWidget* parent) :
	QWidget(parent)
{
	setupUi(this);
	connect(addEncoder, SIGNAL(clicked()), this, SLOT(addEncoderSlot()));
	connect(removeEncoder, SIGNAL(clicked()), this, SLOT(removeEncoderSlot()));
	connect(configureEncoder, SIGNAL(clicked()), this, SLOT(configureEncoderSlot()));
	connect(defaultButton, SIGNAL(clicked()), this, SLOT(setDefaultEncoderSlot()));
	connect(currentEncoderList, SIGNAL(itemDoubleClicked(QListWidgetItem *)),this, SLOT(configureEncoderSlot()));
	connect(encoderWizardButton, SIGNAL(clicked()), this, SLOT(encoderWizard()));

	if (KStandardDirs::findExe("nice") == QString())
		niceLevelBox->setEnabled(false);

	loadEncoderList();
}

/**
 * Clear listbox
 * Load list of encoders.
 */ 
void EncoderConfigImp::loadEncoderList()
{
	currentEncoderList->clear();

	QStringList list = EncoderPrefs::prefsList();
	foreach (QString encoder, list) {
		currentEncoderList->addItem(new QListWidgetItem(encoder.remove("Encoder_")));
	}

	// Make sure that the current encoder is valid.
	QString groupName = QString("Encoder_").append(Prefs::defaultEncoder());
	if (!list.contains(groupName) && currentEncoderList->count() > 0) {
		currentEncoderList->setCurrentRow(0);
		Prefs::setDefaultEncoder((currentEncoderList->currentItem())->text());
	}
	kcfg_defaultEncoder->setText(Prefs::defaultEncoder());
}

void EncoderConfigImp::setDefaultEncoderSlot()
{
	if (currentEncoderList->currentItem())
		kcfg_defaultEncoder->setText((currentEncoderList->currentItem())->text());
}

void EncoderConfigImp::addEncoderSlot()
{
	QString groupName;
	do {
		groupName = QString("__new encoder__").append(KRandom::randomString(10));
	 } while (KConfigDialog::exists(groupName));
 
	KConfigDialog *dialog = new KConfigDialog(this, groupName, EncoderPrefs::prefs(groupName));
	dialog->setFaceType(KPageDialog::Plain);
	dialog->setButtons(KDialog::Ok | KDialog::Cancel | KDialog::Help);
	dialog->setCaption(i18n("Configure Encoder"));
	dialog->addPage(new EncoderEdit(0), i18n("Encoder Configuration"), "package_settings");
	connect(dialog, SIGNAL(settingsChanged(const QString &)), this, SLOT(saveNewEncoderSlot(const QString &)));
	dialog->show();
}


void EncoderConfigImp::saveNewEncoderSlot(const QString &dialogName)
{
	QString encoderName = (EncoderPrefs::prefs(dialogName))->encoderName();
	QString command = EncoderPrefs::prefs(dialogName)->commandLine();
	QString extension = EncoderPrefs::prefs(dialogName)->extension();
	int percentLength = EncoderPrefs::prefs(dialogName)->percentLength();
	KConfigDialog::exists(dialogName)->deleteLater();
	EncoderPrefs::deletePrefs(dialogName);

	if (checkEncoderName(encoderName)) {
		EncoderPrefs *encPrefs;
		encPrefs = EncoderPrefs::prefs(QString("Encoder_").append(encoderName));
		encPrefs->setEncoderName(encoderName);
		encPrefs->setCommandLine(command);
		encPrefs->setExtension(extension);
		encPrefs->setPercentLength(percentLength);
		encPrefs->writeConfig();
		currentEncoderList->addItem(new QListWidgetItem(encoderName));
	} else {
		KMessageBox::error(this, i18n("An encoder with that name already exists, please choose a different one."), i18n("Encoder exists already"), KMessageBox::PlainCaption);

		// Would be easier but doesn't work for me?? GF
		//KConfigDialog::showDialog(dialogName);

		QString groupName;
		do {
			groupName = QString("__new encoder__").append(KRandom::randomString(10));
		} while (KConfigDialog::exists(groupName));
		EncoderPrefs *encPrefs;
		encPrefs = EncoderPrefs::prefs(groupName);
		encPrefs->setEncoderName(QString());
		encPrefs->setCommandLine(command);
		encPrefs->setExtension(extension);
		encPrefs->setPercentLength(percentLength);

		KConfigDialog *dialog = new KConfigDialog(this, groupName, EncoderPrefs::prefs(groupName));
		dialog->setFaceType(KPageDialog::Plain);
		dialog->setButtons(KDialog::Ok | KDialog::Cancel | KDialog::Help);
		dialog->setCaption(i18n("Configure Encoder"));
		EncoderEdit *page = new EncoderEdit(0);
		dialog->addPage(page, i18n("Encoder Configuration"), "package_settings");
		connect(dialog, SIGNAL(settingsChanged(const QString &)), this, SLOT(saveNewEncoderSlot(const QString &)));
		dialog->show();
		page->kcfg_encoderName->insert(encoderName);
	}
}

/**
 * If
 * Something is selected
 * There is more then 1 thing left
 * The user says ok to delete.
 * Then
 * The group is removed from the list
 * Deleted from the config.
 */ 
void EncoderConfigImp::removeEncoderSlot()
{
	if(!currentEncoderList->currentItem()){
		KMessageBox:: sorry(this, i18n("Please select an encoder."), i18n("No Encoder Selected"));
		return;
	}

	if(currentEncoderList->count() <= 1){
		KMessageBox:: sorry(this, i18n("At least one encoder must exist."), i18n("Can Not Remove"));
		return;
	}

	if(KMessageBox::warningContinueCancel(this, i18n("Delete encoder?"), i18n("Delete Encoder"),KStandardGuiItem::del())
		== KMessageBox::Cancel )
		return;

	QString listText = (currentEncoderList->currentItem())->text();
	QString groupName = QString("Encoder_").append(listText);
	KConfigDialog::exists(listText)->deleteLater ();
	EncoderPrefs::deletePrefs(groupName);
	QListWidgetItem *removedItem = currentEncoderList->takeItem(currentEncoderList->currentRow());
	delete removedItem;

	if (listText == Prefs::defaultEncoder()) {
		currentEncoderList->setCurrentRow(0);
		Prefs::setDefaultEncoder((currentEncoderList->currentItem())->text());
	}

	Prefs::self()->writeConfig();
	kcfg_defaultEncoder->setText(Prefs::defaultEncoder());
	emit encoderChanged();
}

/**
 * If
 * Something is selected
 * Group exists
 * Then
 * Bring up dialog
 */ 
void EncoderConfigImp::configureEncoderSlot()
{
	if(!currentEncoderList->currentItem()){
		KMessageBox:: sorry(this, i18n("Please select an encoder."), i18n("No Encoder Selected"));
		return;
	}

	QString listText = (currentEncoderList->currentItem())->text();
	QString groupName = QString("Encoder_").append(listText);
	KConfig &config = *KGlobal::config();
	if(!config.hasGroup(groupName))
		return;

	if(KConfigDialog::showDialog(listText))
		return;

	KConfigDialog *dialog = new KConfigDialog(this, listText, EncoderPrefs::prefs(groupName));
	dialog->setFaceType(KPageDialog::Plain);
	dialog->setButtons(KDialog::Ok | KDialog::Cancel | KDialog::Help);
	dialog->setCaption(i18n("Configure Encoder"));
	dialog->addPage(new EncoderEdit(0), i18n("Encoder Configuration"), "package_settings");
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
 * If current encoder update also.
 */ 
void EncoderConfigImp::updateEncoder(const QString &dialogName)
{
	QString groupName = QString("Encoder_").append(dialogName);
	QString encoderName = (EncoderPrefs::prefs(groupName))->encoderName();
	// if the name changed copy to new name
	if (encoderName != dialogName) {
		EncoderPrefs *encPrefs;
		encPrefs = EncoderPrefs::prefs(QString("Encoder_").append(encoderName));
		encPrefs->setEncoderName(encoderName);
		encPrefs->setCommandLine((EncoderPrefs::prefs(groupName))->commandLine());
		encPrefs->setExtension((EncoderPrefs::prefs(groupName))->extension());
		encPrefs->setPercentLength((EncoderPrefs::prefs(groupName))->percentLength());
		encPrefs->writeConfig();

		//delete old encoder
		KConfigDialog::exists(groupName)->deleteLater();
		EncoderPrefs::deletePrefs(groupName);
		QList<QListWidgetItem *> items = currentEncoderList->findItems(dialogName, Qt::MatchExactly); //should be only one
		QListWidgetItem *removedItem = currentEncoderList->takeItem(currentEncoderList->row(items.at(0)));
		delete removedItem;
		currentEncoderList->addItem(new QListWidgetItem(encoderName));
		if (dialogName == Prefs::defaultEncoder()) {
			Prefs::setDefaultEncoder(encoderName);
		}

		kcfg_defaultEncoder->setText(Prefs::defaultEncoder());
	}

	Prefs::self()->writeConfig();
	emit encoderChanged();
}

bool EncoderConfigImp::checkEncoderName(const QString &encoder)
{
	QStringList list = EncoderPrefs::prefsList();
	if (list.contains(QString("Encoder_").append(encoder))) {
		return false;
	}

	return true;
}

/**
 * Load up the wizard with the encoder fileFormat string.  Save it if OK is hit.
 */
void EncoderConfigImp::encoderWizard()
{
	fileWizard wizard(this);
	wizard.fileFormat->setText(kcfg_fileFormat->text());

	// Show dialog and save results if ok is pressed.
	bool okClicked = wizard.exec();
	if (okClicked) {
		kcfg_fileFormat->setText(wizard.fileFormat->text());
	}
}

/**
 * EncoderPrefs
 */
QHash<QString, EncoderPrefs *> *EncoderPrefs::m_prefs = 0;

EncoderPrefs *EncoderPrefs::prefs(const QString &groupName)
{
	if (!m_prefs) {
		m_prefs = new QHash<QString, EncoderPrefs *>;
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

QStringList EncoderPrefs::prefsList()
{
	KConfig &config = *KGlobal::config();
	QStringList list = config.groupList();
	return list.filter("Encoder_");
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

