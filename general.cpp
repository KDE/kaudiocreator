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

#include <QRegExp>
#include <QLineEdit>
#include <QLabel>

General::General( QWidget* parent )
    : QWidget( parent ), Ui_General()
{
    setupUi( this );
    connect(kcfg_replaceInput,SIGNAL(textChanged(const QString &)),this, SLOT(updateExample()));
    connect(kcfg_replaceOutput, SIGNAL(textChanged(const QString&)), this,SLOT(updateExample()));
    connect(example, SIGNAL(textChanged(const QString&)), this, SLOT(updateExample()));

}

void General::updateExample()
{
  QString text = example->text();
  text.replace( QRegExp(kcfg_replaceInput->text()), kcfg_replaceOutput->text() );
  exampleOutput->setText(text);
}
#include "general.moc"
