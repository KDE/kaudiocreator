/**
 * Copyright (C) 2009 Gerd Fleischer (gerdfleischer at web dot de)
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


#include <Qt>

static const int EMPTY_TRACK = 0;
static const int EMPTY_YEAR = 999;

// static const int JOB_ERROR = -1;
// static const int JOB_COMPLETED = 101;

// jobqueue
static const int HEADER_JOB = 0;
static const int HEADER_PROGRESS = 1;
static const int HEADER_DESCRIPTION = 2;
// static const int ICON_LOC = HEADER_DESCRIPTION;

static const int DEFAULT_HIGHEST_NUMBER = 9;

enum DataRole {
    PercentDone = Qt::UserRole + 1,
    JobState,
    JobId
};

enum JobState {
    JOB_QUEUED = 0,
    JOB_PROGRESSING = 1,
    JOB_COMPLETED = 101,
    JOB_ERROR = -1
};

// tracks
static const int COLUMN_RIP = 0;
static const int COLUMN_TRACK = 1;
static const int COLUMN_LENGTH = 2;
static const int COLUMN_TRACK_NAME = 3;
static const int COLUMN_TRACK_ARTIST = 4;
static const int COLUMN_TRACK_COMMENT = 5;

