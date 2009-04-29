// TODO insert license here
#ifndef GENERAL_H
#define GENERAL_H

#include "ui_general.h"


class General : public QWidget, private Ui::General
{
Q_OBJECT

public:
	General( QWidget* parent = 0 );

public slots:
	void updateExample();
   
private:
	const QString make_fat32_compatible(const QString &);
};

#endif
