/*   This file is part of KatePlugin-IndexView
 *
 *   BashParser Class
 *   Copyright (C) 2018 loh.tar@googlemail.com
 *
 *   Inspired by bash_parser.cpp, part of Kate's SymbolViewer
 *   Copyright (C) 2008 Daniel Dumitrache <daniel.dumitrache@gmail.com>
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

#include <KLocalizedString>

#include "icon_collection.h"
#include "index_view.h"

#include "bash_parser.h"

BashParser::BashParser(IndexView *view)
    : ProgramParser(view)
{
    using namespace IconCollection;
    registerViewOption(FunctionNode, FunctionIcon, QStringLiteral("Functions"), i18n("Show Functions"));
//     registerViewOption(QStringLiteral("Parameters"), i18n("Show Parameter"));

    m_nestableElements << FunctionNode;

    initHereDoc(QStringLiteral("<<-?"), QStringLiteral("\"'"));
}


BashParser::~BashParser()
{
}


void BashParser::parseDocument()
{
    while (nextInstruction()) {
        if (m_line.startsWith(QStringLiteral("function "))) {
            m_line = m_line.mid(9);
            m_line = m_line.section(QRegularExpression(QStringLiteral("\\W")), 0, 0);
            addNode(FunctionNode, m_line, m_lineNumber);

        } else if (m_line.contains(QRegularExpression(QStringLiteral("^\\w+ *\\( *\\)")))) {
            m_line = m_line.section(QRegularExpression(QStringLiteral("\\W")), 0, 0);
            addNode(FunctionNode, m_line, m_lineNumber);
        }
    }
}


void BashParser::removeComment()
{
    // Remove parameter expansion with braces or they will broken up by
    // removeTrailingSharpComment when there is a sharp included
    m_line.remove(QRegularExpression(QStringLiteral("\\$\\{.*\\}")));

    removeTrailingSharpComment();

    // Remove math expressions in double parentheses because there could be
    // the shift operator which would cause false detection of heredoc oprerator
    m_line.remove(QRegularExpression(QStringLiteral("\\(\\(.*\\)\\)")));
    // Remove here string operator which would cause false detection of heredoc oprerator
    m_line.remove(QStringLiteral("<<<"));
    removeHereDoc();
}

// kate: space-indent on; indent-width 4; replace-tabs on;
