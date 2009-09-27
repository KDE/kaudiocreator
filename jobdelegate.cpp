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

#include "jobdelegate.h"

#include <QApplication>

JobDelegate::JobDelegate(QObject *parent) : QStyledItemDelegate(parent)
{
}

void JobDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if (index.column() == 1) {
        QStyleOptionProgressBarV2 opts;
        opts.palette = option.palette;
        opts.rect = option.rect;
        opts.direction = option.direction;
        opts.minimum = 0;
        opts.maximum = 100;
        opts.progress = index.model()->data(index, Qt::DisplayRole).toInt();
        QApplication::style()->drawControl(QStyle::CE_ProgressBar, &opts, painter, 0);
    } else {
        QStyledItemDelegate::paint(painter, option, index);
    }
}

#include "jobdelegate.moc"
