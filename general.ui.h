/****************************************************************************
** ui.h extension file, included from the uic-generated form implementation.
**
** If you wish to add, delete or rename functions or slots use
** Qt Designer which will update this file, preserving your code. Create an
** init() function in place of a constructor, and a destroy() function in
** place of a destructor.
*****************************************************************************/

#include <qregexp.h>
#include <qlineedit.h>
#include <qlabel.h>

void General::updateExample()
{
  QString text = example->text();
  text.replace( QRegExp(kcfg_replaceInput->text()), kcfg_replaceOutput->text() );
  exampleOutput->setText(text);
}
