
/* BEGIN_COMMON_COPYRIGHT_HEADER
 *
 * TOra - An Oracle Toolkit for DBA's and developers
 *
 * Shared/mixed copyright is held throughout files in this product
 *
 * Portions Copyright (C) 2000-2001 Underscore AB
 * Portions Copyright (C) 2003-2005 Quest Software, Inc.
 * Portions Copyright (C) 2004-2013 Numerous Other Contributors
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation;  only version 2 of
 * the License is valid for this program.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program as the file COPYING.txt; if not, please see
 * http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt.
 *
 *      As a special exception, you have permission to link this program
 *      with the Oracle Client libraries and distribute executables, as long
 *      as you follow the requirements of the GNU GPL in regard to all of the
 *      software in the executable aside from Oracle client libraries.
 *
 * All trademarks belong to their respective owners.
 *
 * END_COMMON_COPYRIGHT_HEADER */

#include "views/toplaintextview.h"

#include <QtGui/QFocusEvent>
#include <QtGui/QTextDocument>
#include <QPlainTextEdit>

#include "widgets/tosearchreplace.h"
#include "core/utils.h"

using namespace Views;

toPlainTextView::toPlainTextView(QWidget *parent /* = 0*/, const char *name /* = 0*/)
    : super(parent)
    , toEditWidget()
    , m_model(NULL)
    , m_model_column(0)
{
    if (name)
        setObjectName(name);

    toEditWidget::FlagSet.Save = true;
    toEditWidget::FlagSet.Paste = false;
    toEditWidget::FlagSet.SelectAll = true;
    toEditWidget::FlagSet.SelectBlock = false;

    m_view = new QPlainTextEdit(this);
    m_view->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);

    QVBoxLayout *l = new QVBoxLayout();
    l->setSpacing(0);
    l->setContentsMargins(0, 0, 0, 0);
    l->addWidget(m_view);
    setLayout(l);
}

void toPlainTextView::setReadOnly(bool ro)
{
    m_view->setReadOnly(ro);
}

void toPlainTextView::setText(const QString &t)
{
    m_view->setPlainText(t);
}

void toPlainTextView::setFilename(const QString &f)
{
    m_filename = f;
}

bool toPlainTextView::editSave(bool)
{
    QString fn = Utils::toSaveFilename(m_filename, QString::fromLatin1("*.txt"), this);
    if (!fn.isEmpty())
    {
        return Utils::toWriteFile(fn, m_view->toPlainText());
    }
    return false;
}

QString toPlainTextView::editText()
{
    return m_view->toPlainText();
}

void toPlainTextView::editCopy(void)
{
    m_view->copy();
}

void toPlainTextView::editSelectAll(void)
{
    m_view->selectAll();
}

void toPlainTextView::focusInEvent(QFocusEvent *e)
{
    super::focusInEvent(e);
    toEditWidget::gotFocus();
}

void toPlainTextView::focusOutEvent(QFocusEvent *e)
{
    super::focusOutEvent(e);
    toEditWidget::lostFocus();
}

void toPlainTextView::setFont(const QFont &f)
{
    m_view->setFont(f);
}

const QFont& toPlainTextView::font()
{
    return m_view->font();
}

void toPlainTextView::setModel(QAbstractItemModel *model)
{
    m_model = model;
    modelReset();
    connect(model, SIGNAL(modelReset()), this, SLOT(modelReset()));
    connect(model, SIGNAL(rowsInserted(const QModelIndex &, int , int)), this, SLOT(rowsInserted(const QModelIndex &, int, int)));
    connect(model, SIGNAL(rowsRemoved(const QModelIndex &, int, int)), this, SLOT(rowsRemoved(const QModelIndex &, int, int)));
    setReadOnly(true);
}

void toPlainTextView::modelReset()
{
    m_lines.clear();
    m_view->clear();
    for(int row = 0; row < m_model->rowCount(); row++)
    {
        QModelIndex index = m_model->index(row, m_model_column);
        m_view->appendPlainText(m_model->data(index).toString());
    }
}

void toPlainTextView::rowsInserted(const QModelIndex &parent, int first, int last)
{
    if (!m_lines.contains(first))
    {
        for(int row = first; row <= last; row++)
        {
            m_lines.insert(row);
            QModelIndex index = m_model->index(row, m_model_column);
            m_view->appendPlainText(m_model->data(index).toString());
        }
    }
}

void toPlainTextView::rowsRemoved(const QModelIndex &parent, int first, int last)
{
    for (int row = first; row <= last; row++)
    {
        m_lines.remove(row);
    }
}

bool toPlainTextView::handleSearching(QString const& search, QString const& replace, Search::SearchFlags flags)
{
    QTextDocument::FindFlags f;
    if (flags & Search::WholeWords)
        f |= QTextDocument::FindWholeWords;
    if (flags & Search::CaseSensitive)
        f |= QTextDocument::FindCaseSensitively;

    return m_view->find(search, f);
}
