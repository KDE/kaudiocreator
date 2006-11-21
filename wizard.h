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
    
public slots:
    virtual void extensionPressed();
    virtual void trackArtistPressed();
    virtual void trackCommentPressed();
    virtual void genrePressed();
    virtual void albumPressed();
    virtual void artistPressed();
    virtual void commentPressed();
    virtual void fileFormatTextChanged( const QString & text );

private slots:
    virtual void homePressed();
    virtual void trackTitlePressed();
    virtual void trackNumberPressed();
    virtual void yearPressed();


};

#endif
