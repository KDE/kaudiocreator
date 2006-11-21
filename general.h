// TODO insert license here
#ifndef GENERAL_H
#define GENERAL_H

#include "ui_general.h"


class General : public QWidget, private Ui_General
{
    Q_OBJECT

public:
    General( QWidget* parent = 0 );
    

public slots:
   void updateExample();
};

#endif
