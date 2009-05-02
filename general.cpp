#include "general.h"
// TODO replace comment below with license
/****************************************************************************
** ui.h extension file, included from the uic-generated form implementation.
**
** If you wish to add, delete or rename functions or slots use
** Qt Designer which will update this file, preserving your code. Create an
** init() function in place of a constructor, and a destroy() function in
** place of a destructor.
*****************************************************************************/

#include "job.h"

#include <kmessagebox.h>

#include <QRegExp>

General::General( QWidget* parent )
	: QWidget( parent )
{
	setupUi( this );
	connect(kcfg_replaceInput,SIGNAL(textChanged(const QString &)),this, SLOT(updateExample()));
	connect(kcfg_replaceOutput, SIGNAL(textChanged(const QString &)), this,SLOT(updateExample()));
	connect(example, SIGNAL(textChanged(const QString &)), this, SLOT(updateExample()));
	connect(kcfg_fat32compatible, SIGNAL(toggled(bool)), this, SLOT(updateExample()));
	connect(kcfg_replaceFatIncompatible, SIGNAL(textChanged(const QString &)), this,SLOT(check_fat32_compatibility(const QString &)));
}

void General::updateExample()
{
	QString text = example->text();
	text.replace( QRegExp(kcfg_replaceInput->text()), kcfg_replaceOutput->text() );
	exampleOutput->setText(text);
}

void General::check_fat32_compatibility(const QString &text)
{
	bool containsNonFat = false;
	QString rep = text;
	QStringList wrongParts, list;
	list << "\\" << "/" << ":" << "*" << "?" << "<" << ">" << "|";
	
	foreach (QString nonFat, list) {
		if (rep.contains(nonFat)) {
			containsNonFat = true;
			wrongParts << nonFat;
		}
	}
	
	if (containsNonFat) {
		KMessageBox::information(this, i18n("The replacement string still contains incompatible characters (%1).\nCopying to FAT32 might not work as expected.").arg(wrongParts.join(", ")));
	}
}

#include "general.moc"
