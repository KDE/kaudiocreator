/****************************************************************************
** ui.h extension file, included from the uic-generated form implementation.
**
** If you wish to add, delete or rename functions or slots use
** Qt Designer which will update this file, preserving your code. Create an
** init() function in place of a constructor, and a destroy() function in
** place of a destructor.
*****************************************************************************/

#include "wizard.h"
#include <qlineedit.h>

/**
 * Load up the wizard with the playlist string.  Save it if OK is hit.
 */
void EncoderConfig::playlistWizard(){
  fileWizard wizard(this, "Playlist File FormatWizard", true);
  wizard.playlistFormat->setText(kcfg_playlistFileFormat->text());
  // Show dialog and save results if ok is pressed.
  bool okClicked = wizard.exec();
  if(okClicked){
    kcfg_playlistFileFormat->setText(wizard.playlistFormat->text());
  }
}

/**
 * Load up the wizard with the encoder playlist string.  Save it if OK is hit.
 */
void EncoderConfig::encoderWizard(){
  fileWizard wizard(this, "Encoder File Format Wizard", true);
  wizard.playlistFormat->setText(kcfg_fileFormat->text());

  // Show dialog and save results if ok is pressed.
  bool okClicked = wizard.exec();
  if(okClicked){
    kcfg_fileFormat->setText(wizard.playlistFormat->text());
  }
}
