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

#ifndef WIZARD_H
#define WIZARD_H

#include "ui_wizard.h"


class fileWizard : public QDialog, public Ui_fileWizard
{
    Q_OBJECT

public:
    fileWizard( QWidget* parent = 0 );
    
private slots:
    void homePressed();
    void trackTitlePressed();
    void trackNumberPressed();
    void yearPressed();
    void extensionPressed();
    void trackArtistPressed();
    void trackCommentPressed();
    void genrePressed();
    void albumPressed();
    void artistPressed();
    void commentPressed();
    void artistInitialPressed();
    void fileFormatTextChanged( const QString & text );
};

#endif
