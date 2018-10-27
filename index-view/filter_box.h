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


#ifndef FILTER_BOX_H
#define FILTER_BOX_H

#include <QComboBox>

class IndexView;
class KatePluginIndexView;

class FilterBox : public QComboBox
{
//     Q_OBJECT

public:
    enum Indicator {
        Neutral,
        Match,
        NoMatch
    };

     FilterBox(IndexView *view, KatePluginIndexView *plugin);
    ~FilterBox() override;

    void focusInEvent(QFocusEvent *e) override;
    void focusOutEvent(QFocusEvent *e) override;

    void indicateMatch(Indicator i);

protected:
    IndexView *m_view;
};

#endif

// kate: space-indent on; indent-width 4; replace-tabs on;
