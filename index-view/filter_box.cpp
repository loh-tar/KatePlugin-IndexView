/*   This file is part of KatePlugin-IndexView
 *
 *   FilterBox Class
 *   Copyright (C) 2018 loh.tar@googlemail.com
 *
 *   This library is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU Library General Public
 *   License as published by the Free Software Foundation; either
 *   version 2 of the License, or (at your option) any later version.
 *
 *   This library is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *   Library General Public License for more details.
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this library; if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */


#include <QLineEdit>

#include <KLocalizedString>
#include <KTextEditor/View>
#include <KColorScheme>

#include "index_view.h"
#include "kate_plugin_index_view.h"

#include "filter_box.h"


FilterBox::FilterBox(IndexView *view, KatePluginIndexView *plugin)
    : QComboBox(nullptr)
    , m_view(view)
{
    setEditable(true);
    setEditable(true);
    setInsertPolicy(QComboBox::InsertAtTop);
    setMaxCount(9);
    setMaxVisibleItems(5);
    // Accept only pattern without space
    static const QRegularExpression rx(QStringLiteral("^\\S+$"));
    setValidator(new QRegularExpressionValidator(rx, this));
    lineEdit()->setClearButtonEnabled(true);
    lineEdit()->setPlaceholderText(i18n("%1 Filter", plugin->name()));
}


FilterBox::~FilterBox()
{}


void FilterBox::focusInEvent(QFocusEvent *e)
{
    KTextEditor::View *docView = m_view->m_mainWindow->activeView();
    if (!docView) {
        return;
    }

    // Remove any selection so that IndexView::filterTree() will not use it
    docView->setSelection(KTextEditor::Range());
    QComboBox::focusInEvent(e);
};


void FilterBox::focusOutEvent(QFocusEvent *e)
{
    // Add the current pattern to the history list but avoid duplicates
    // FIXME: Missed I something or is there no default setting for such behavior?
    // Only after "return" will the pattern added by default
    QString pattern = lineEdit()->text();
    if (!pattern.isEmpty()) {
        removeItem(findText(pattern, Qt::MatchFixedString));
        insertItem(0, pattern.toLower());
        setCurrentIndex(0);
    }

    QComboBox::focusOutEvent(e);
}


void FilterBox::indicateMatch(Indicator i)
{
    // Almost Copy&Paste from KatePluginSearchView::indicateMatch, Thanks!
    QPalette background(lineEdit()->palette());

    switch (i) {
        case Neutral:
            KColorScheme::adjustBackground(background, KColorScheme::NormalBackground);
            break;
        case Match:
            KColorScheme::adjustBackground(background, KColorScheme::PositiveBackground);
            break;
        case NoMatch:
            KColorScheme::adjustBackground(background, KColorScheme::NegativeBackground);
            break;
    }

    lineEdit()->setPalette(background);
}

// kate: space-indent on; indent-width 4; replace-tabs on;
