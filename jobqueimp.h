/**
 * This file is part of the KAudioCreator package
 * Copyright (C) 2003 Benjamin C Meyer (ben+kaudiocreator at meyerhome dot net)
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

#ifndef JOBQUEIMP_H
#define JOBQUEIMP_H

#include "jobque.h"
#include <tqstring.h>
#include <tqlistview.h>

class Job;
class TQPainter;
class TQColorGroup;
class TQListView;

/**
 * Helper class to allow for progress bars in list view items.
 */
class QueListViewItem : public TQListViewItem {

public:
	QueListViewItem (TQListView * p = NULL, const TQString a=0, const TQString b=0, const TQString c=0, const TQString d=0, const TQString e=0);
	virtual void paintCell (TQPainter * p,const TQColorGroup &cg,int column,
      int width,int align);
	double percentDone;
	// Has the percentDone gone beyond 0
	// Here because percentDone might go 1,2,3,4 or it could go 1,20,21,78 or ...
	bool progressing;
};


class JobQueImp : public JobQue {

Q_OBJECT

signals:
	void removeJob( int idNumber );

public:
	JobQueImp( TQWidget* parent = 0, const char* name = 0 );
	int numberOfJobsNotFinished();

public slots:
	void updateProgress( int id, int progress );
	void addJob( Job* job, const TQString &name );

	// Toolbar Button
	void clearDoneJobs();

private slots:
	void removeSelectedJob();
	void removeAllJobs();

private:
	bool removeJob( QueListViewItem *item, bool kill=true, bool prompt=true );
	TQString getStringFromNumber( int number );
	int highestNumber;

	int currentId;
};

#endif

