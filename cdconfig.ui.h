/****************************************************************************
** ui.h extension file, included from the uic-generated form implementation.
**
** If you wish to add, delete or rename functions or slots use
** Qt Designer which will update this file, preserving your code. Create an
** init() function in place of a constructor, and a destroy() function in
** place of a destructor.
*****************************************************************************/

#include <kmessagebox.h>
#include <kprocess.h>

void CdConfig::configureAudioCD()
{
  KMessageBox::information(this,
    i18n("The CDDA tab is the only one that is utilized at this time."),
    i18n("CDDA Tab"), i18n("CDDA Tab"));
  KShellProcess proc;
  proc << "kcmshell" << "audiocd";
  proc.start(KShellProcess::DontCare,  KShellProcess::NoCommunication);
}
