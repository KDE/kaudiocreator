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

#include <QSet>
#include <QHash>

/**
 * Constructor.
 */
EncoderConfigImp::EncoderConfigImp(QWidget* parent) :
	QWidget(parent)
{
	setupUi(this);
	connect(addEncoder, SIGNAL(clicked()), this, SLOT(addEncoderSlot()));
	connect(copyEncoder, SIGNAL(clicked()), this, SLOT(copyEncoderSlot()));
	connect(removeEncoder, SIGNAL(clicked()), this, SLOT(removeEncoderSlot()));
	connect(configureEncoder, SIGNAL(clicked()), this, SLOT(configureEncoderSlot()));
	connect(defaultButton, SIGNAL(clicked()), this, SLOT(setDefaultEncoderSlot()));
	connect(currentEncoderList, SIGNAL(itemDoubleClicked(QListWidgetItem *)),this, SLOT(configureEncoderSlot()));
	connect(encoderWizardButton, SIGNAL(clicked()), this, SLOT(encoderWizard()));

	if (KStandardDirs::findExe("nice") == QString())
		niceLevelBox->setEnabled(false);

	loadEncoderList();
}

EncoderConfigImp::~EncoderConfigImp()
{
	// delete temporary configs, e.g. from canceled dialogs
	KConfig &config = *KGlobal::config();
	QStringList list = config.groupList().filter("__new encoder__");
	foreach (QString encoder, list) {
		config.deleteGroup(encoder);
	}
	Prefs::self()->writeConfig();
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

	// Make sure that the default encoder is valid.
	QString groupName = QString("Encoder_").append(Prefs::defaultEncoder());
	if (!list.contains(groupName)) {
		Prefs::setDefaultEncoder("WAV");
		Prefs::self()->writeConfig();
	}
	kcfg_defaultEncoder->setText(Prefs::defaultEncoder());
}

void EncoderConfigImp::setDefaultEncoderSlot()
{
	if (currentEncoderList->currentItem()) {
		QString listText = (currentEncoderList->currentItem())->text();
		QString groupName = QString("Encoder_").append(listText);
		QStringList types = EncoderPrefs::prefs(groupName)->inputTypes().split(",", QString::SkipEmptyParts);
		if (types.contains(QString("wav")))
			kcfg_defaultEncoder->setText((currentEncoderList->currentItem())->text());
		else
			KMessageBox:: sorry(this, i18n("The default encoder has to accept wav as input."), i18n("Not Allowed"));
	}
}

void EncoderConfigImp::createInputTypesList()
{
	QStringList encoderInputTypesList, inputTypesList;
	QStringList encoderList = EncoderPrefs::prefsList();
	foreach (QString encoder, encoderList) {
		inputTypesList << EncoderPrefs::prefs(encoder)->inputTypes().split(",", QString::SkipEmptyParts);
	}

	QSet<QString> inputTypesSet = inputTypesList.toSet(); // remove duplicates
	inputTypesList = inputTypesList.fromSet(inputTypesSet);
	inputTypesList.sort();
	Prefs::setInputTypesList(inputTypesList);
}

void EncoderConfigImp::addEncoderSlot()
{
	QString groupName;
	do {
		groupName = QString("__new encoder__").append(KRandom::randomString(10));
	 } while (EncoderEditDialog::exists(groupName));
 
	EncoderEditDialog *dialog = new EncoderEditDialog(this, groupName, EncoderPrefs::prefs(groupName), true);
	connect(dialog, SIGNAL(settingsChanged(const QString &)), this, SLOT(saveNewEncoderSlot(const QString &)));
	dialog->setCaption(i18n("Add Encoder"));
	dialog->show();
}

void EncoderConfigImp::saveNewEncoderSlot(const QString &dialogName)
{
	EncoderPrefs *origPrefs = EncoderPrefs::prefs(dialogName);
	QString encoderName = origPrefs->encoderName();
	EncoderPrefs *encPrefs = cloneEncoder(origPrefs, QString("Encoder_").append(encoderName));
	encPrefs->writeConfig();

	EncoderEditDialog::exists(dialogName)->deleteLater();
	EncoderPrefs::deletePrefs(dialogName);

	currentEncoderList->addItem(new QListWidgetItem(encoderName));
	createInputTypesList();
	Prefs::self()->writeConfig();
}

void EncoderConfigImp::copyEncoderSlot()
{
	if(!currentEncoderList->currentItem()) {
		KMessageBox:: sorry(this, i18n("Please select an encoder."), i18n("No Encoder Selected"));
		return;
	}

	QString listText = (currentEncoderList->currentItem())->text();
	QString groupName = QString("Encoder_").append(listText);
	KConfig &config = *KGlobal::config();
	if(!config.hasGroup(groupName))
		return;

	if(EncoderEditDialog::showDialog(listText))
		return;

	QString tmpEncoderName;
	do {
		tmpEncoderName = QString("__new encoder__").append(KRandom::randomString(10));
	 } while (EncoderEditDialog::exists(tmpEncoderName));

	EncoderPrefs *origPrefs = EncoderPrefs::prefs(groupName);
	EncoderPrefs *encPrefs = cloneEncoder(origPrefs, tmpEncoderName);

	EncoderEditDialog *dialog = new EncoderEditDialog(this, tmpEncoderName, encPrefs, true);
	connect(dialog, SIGNAL(settingsChanged(const QString &)), this, SLOT(saveNewEncoderSlot(const QString &)));
	dialog->setCaption(i18n("Copy Encoder"));
	dialog->show();
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
	if (!currentEncoderList->currentItem()) {
		KMessageBox:: sorry(this, i18n("Please select an encoder."), i18n("No Encoder Selected"));
		return;
	}

	if (currentEncoderList->currentItem()->text() == QString("WAV")) {
		KMessageBox:: sorry(this, i18n("WAV is not allowed to be removed."), i18n("Not Allowed"));
		return;
	}

	if(KMessageBox::warningContinueCancel(this, i18n("Delete encoder?"), i18n("Delete Encoder"),KStandardGuiItem::del())
		== KMessageBox::Cancel )
		return;

	QString listText = (currentEncoderList->currentItem())->text();
	QString groupName = QString("Encoder_").append(listText);
	EncoderEditDialog::exists(listText)->deleteLater ();
	EncoderPrefs::deletePrefs(groupName);
	QListWidgetItem *removedItem = currentEncoderList->takeItem(currentEncoderList->currentRow());
	delete removedItem;

	if (listText == Prefs::defaultEncoder()) {
		currentEncoderList->setCurrentRow(0);
		Prefs::setDefaultEncoder((currentEncoderList->currentItem())->text());
	}

	createInputTypesList();
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

	if(currentEncoderList->currentItem()->text() == QString("WAV")) {
		KMessageBox:: sorry(this, i18n("WAV is not allowed to be changed.\nYou can copy it to adjust it to your needs."), i18n("Not Allowed"));
		return;
	}

	QString listText = (currentEncoderList->currentItem())->text();
	QString groupName = QString("Encoder_").append(listText);
	KConfig &config = *KGlobal::config();
	if(!config.hasGroup(groupName))
		return;

	if(EncoderEditDialog::showDialog(listText))
		return;

	EncoderEditDialog *dialog = new EncoderEditDialog(this, listText, EncoderPrefs::prefs(groupName));
	connect(dialog, SIGNAL(settingsChanged(const QString &)), this, SLOT(updateEncoder(const QString &)));
	dialog->setCaption(i18n("Configure Encoder"));
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
		EncoderPrefs *origPrefs = EncoderPrefs::prefs(groupName);
		EncoderPrefs *encPrefs = cloneEncoder(origPrefs, QString("Encoder_").append(encoderName));
		encPrefs->setEncoderName(encoderName);
		encPrefs->writeConfig();

		//delete old encoder
		EncoderEditDialog::exists(groupName)->deleteLater();
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

	createInputTypesList();
	Prefs::self()->writeConfig();
	emit encoderChanged();
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
 * Helperfunction to copy one EncoderPref to another
 */
EncoderPrefs *EncoderConfigImp::cloneEncoder(EncoderPrefs *origPrefs, const QString &newName)
{
	QString encoderName = origPrefs->encoderName();
	QString command = origPrefs->commandLine();
	QString extension = origPrefs->extension();
	QString inputTypes = origPrefs->inputTypes();
	int percentLength = origPrefs->percentLength();
	bool checkOutput = origPrefs->checkOutput();

	EncoderPrefs *newPrefs = EncoderPrefs::prefs(newName);
	newPrefs->setEncoderName(encoderName);
	newPrefs->setCommandLine(command);
	newPrefs->setExtension(extension);
	newPrefs->setInputTypes(inputTypes);
	newPrefs->setPercentLength(percentLength);
	newPrefs->setCheckOutput(checkOutput);

	return newPrefs;
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

/**
 * EncoderEditDialog
 */
EncoderEditDialog::EncoderEditDialog(QWidget *parent, const QString &name, KConfigSkeleton *config, bool isNew)
	: KConfigDialog(parent, name, config), isNewEncoder(isNew), origName(name)
{
	setFaceType(KPageDialog::Plain);
	setButtons(KDialog::Ok | KDialog::Cancel | KDialog::Help);
	editDialog = new EncoderEdit(0);
	addPage(editDialog, i18n("Encoder Configuration"), "package_settings");
}

void EncoderEditDialog::updateSettings()
{
	// avoid double encodernames
	QString name = editDialog->kcfg_encoderName->text();
	if (isNewEncoder || name != origName) {
		QStringList list = EncoderPrefs::prefsList();
		QString newName = name;
		int i = 2;
		while (list.contains(QString("Encoder_").append(newName))) {
			newName = name + " " + QString::number(i);
			++i;
		}
		editDialog->kcfg_encoderName->setText(newName);
	}

	// trim inputtypes and turn them to lower letters
	QStringList inputTypesList;
	foreach (QString type, editDialog->kcfg_inputTypes->text().split(",", QString::SkipEmptyParts)) {
		inputTypesList << type.trimmed().toLower();
	}
	QString inputTypes = inputTypesList.join(",");
	editDialog->kcfg_inputTypes->setText(inputTypes);

	KConfigDialog::updateSettings();
}

#include "encoderconfigimp.moc"

