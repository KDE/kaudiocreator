// TODO insert license here
#ifndef GENERAL_H
#define GENERAL_H

#include "ui_general.h"

class General : public QWidget, private Ui::General
{
Q_OBJECT

public:
	General( QWidget* parent = 0 );

private slots:
	void updateExample();
	void check_fat32_compatibility(const QString &);
};

#endif
