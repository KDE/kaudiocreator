/****************************************************************************
** Form interface generated from reading ui file './cdconfig.ui'
**
** Created: Wed Feb 13 09:47:42 2002
**      by:  The User Interface Compiler (uic)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/
#ifndef CDCONFIG_H
#define CDCONFIG_H

#include <qvariant.h>
#include <qwidget.h>
class QVBoxLayout; 
class QHBoxLayout; 
class QGridLayout; 
class QCheckBox;
class QFrame;
class QGroupBox;
class QLabel;
class QLineEdit;
class QSpinBox;

class CdConfig : public QWidget
{ 
    Q_OBJECT

public:
    CdConfig( QWidget* parent = 0, const char* name = 0, WFlags fl = 0 );
    ~CdConfig();

    QCheckBox* performCDDBauto;
    QGroupBox* GroupBox1;
    QLabel* TextLabel3;
    QLineEdit* databaseServer;
    QSpinBox* databasePort;
    QLabel* TextLabel2;
    QFrame* Line1;
    QCheckBox* autoRip;

protected:
    QVBoxLayout* CdConfigLayout;
    QGridLayout* GroupBox1Layout;
};

#endif // CDCONFIG_H
