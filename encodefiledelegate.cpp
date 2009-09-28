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

#include "encodefiledelegate.h"
#include "encodefileimp.h"

#include <QDate>

EncodeFileDelegate::EncodeFileDelegate(QObject *parent) : QStyledItemDelegate(parent)
{
}

QWidget *EncodeFileDelegate::createEditor(QWidget *parent,
     const QStyleOptionViewItem &/* option */,
     const QModelIndex &index) const
{
    int column = index.column();
    if (column == COLUMN_GENRE) {
        KComboBox *editor = new KComboBox(parent);
        editor->addItems(index.model()->data(index, Qt::UserRole).toStringList());
        return editor;
    } else if (column == COLUMN_TRACK) {
        QSpinBox *editor = new QSpinBox(parent);
        editor->setMinimum(1);
        return editor;
    } else if (column == COLUMN_YEAR) {
        QSpinBox *editor = new QSpinBox(parent);
        editor->setMinimum(0);
        editor->setMaximum(QDate::currentDate().year());
        return editor;
    } else if (column == COLUMN_ENCODER) {
        KComboBox *editor = new KComboBox(parent);
        editor->addItems(index.model()->data(index, Qt::UserRole + 1).toStringList());
        return editor;
    } else {
        QLineEdit *editor = new QLineEdit(parent);
        return editor;
    }
}

void EncodeFileDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    int column = index.column();
    if (column == COLUMN_GENRE || column == COLUMN_ENCODER) {
        KComboBox *comboBox = static_cast<KComboBox *>(editor);
        comboBox->setEditText(index.model()->data(index, Qt::DisplayRole).toString());
    } else if (column == COLUMN_TRACK || column == COLUMN_YEAR) {
        QSpinBox *spinBox = static_cast<QSpinBox *>(editor);
        spinBox->setValue(index.model()->data(index, Qt::DisplayRole).toString().toInt());
    } else {
        QLineEdit *textEdit = static_cast<QLineEdit *>(editor);
        textEdit->setText(index.model()->data(index, Qt::DisplayRole).toString());
    }
}

void EncodeFileDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
                                      const QModelIndex &index) const
{
    int column = index.column();
    if (column == COLUMN_GENRE || column == COLUMN_ENCODER) {
        KComboBox *comboBox = static_cast<KComboBox *>(editor);
        model->setData(index, comboBox->currentText(), Qt::DisplayRole);
    } else if (column == COLUMN_TRACK || column == COLUMN_YEAR) {
        QSpinBox *spinBox = static_cast<QSpinBox *>(editor);
        model->setData(index, QString::number(spinBox->value()), Qt::DisplayRole);
    } else {
        QLineEdit *textEdit = static_cast<QLineEdit *>(editor);
        model->setData(index, textEdit->text(), Qt::DisplayRole);
    }
}

void EncodeFileDelegate::updateEditorGeometry(QWidget *editor,
    const QStyleOptionViewItem &option, const QModelIndex &/* index */) const
{
    editor->setGeometry(option.rect);
}

QSize EncodeFileDelegate::sizeHint(const QStyleOptionViewItem &/*option*/, const QModelIndex &index) const
{
    QString text = index.data(Qt::DisplayRole).toString();
    return QSize(QApplication::fontMetrics().width(text), QApplication::fontMetrics().height() * 1.4);
}

#include "encodefiledelegate.moc"
