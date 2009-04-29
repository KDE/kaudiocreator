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

#include <QRegExp>

General::General( QWidget* parent )
	: QWidget( parent )
{
	setupUi( this );
	connect(kcfg_replaceInput,SIGNAL(textChanged(const QString &)),this, SLOT(updateExample()));
	connect(kcfg_replaceOutput, SIGNAL(textChanged(const QString &)), this,SLOT(updateExample()));
	connect(example, SIGNAL(textChanged(const QString &)), this, SLOT(updateExample()));
	connect(kcfg_fat32compatible, SIGNAL(toggled(bool)), this, SLOT(updateExample()));
	connect(kcfg_replaceFatIncompatible, SIGNAL(textChanged(const QString &)), this,SLOT(updateExample()));
}

void General::updateExample()
{
	QString text = example->text();
	text.replace( QRegExp(kcfg_replaceInput->text()), kcfg_replaceOutput->text() );
	if (kcfg_fat32compatible->isChecked()) {
		text = make_fat32_compatible(text);
	}
	exampleOutput->setText(text);
}

const QString General::make_fat32_compatible(const QString &text)
{
	QString s = text;
	QString newPattern = kcfg_replaceFatIncompatible->text();
	s.replace("\\", newPattern);
	s.replace("/", newPattern);
	s.replace(":", newPattern);
	s.replace("*", newPattern);
	s.replace("?", newPattern);
	s.replace("<", newPattern);
	s.replace(">", newPattern);
	s.replace("|", newPattern);
	return s;
}

#include "general.moc"
