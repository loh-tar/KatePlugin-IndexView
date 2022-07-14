/*   This file is part of KatePlugin-IndexView
 *
 *   TclParser Class
 *   Copyright (C) 2018 loh.tar@googlemail.com
 *
 *   Inspired by tcl_parser.cpp, part of Kate's SymbolViewer
 *   Copyright (C) 2003 Massimo Callegari <massimocallegari@yahoo.it>
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

#include "tcl_parser.h"


TclParser::TclParser(IndexView *view)
    : ProgramParser(view)
{
    using namespace IconCollection;
    registerViewOption(VariableNode, VariableIcon, QStringLiteral("Variables"), i18n("Show Variables"));
    m_showAssignments = registerViewOptionModifier(VariableNode, QStringLiteral("Assignments"), i18n("Show Assignments"));
    registerViewOption(FunctionNode, FuncDefIcon, QStringLiteral("Functions"), i18n("Show Functions"));
    m_showParameters  = registerViewOptionModifier(FunctionNode, QStringLiteral("Parameters"), i18n("Show Parameter"));

    m_nonBlockElements << VariableNode;

    m_rxVariable = QRegularExpression(QStringLiteral("^set (\\w+)\\{?([^;]*)?\\}?"));
    m_rxFunction = QRegularExpression(QStringLiteral("^proc (\\w+)\\{?([\\w\\s]*)?\\}?"));
}


TclParser::~TclParser()
{
}


void TclParser::parseDocument()
{
    QRegularExpressionMatch rxMatch;

    while (nextInstruction()) {
        // Let's start the investigation
        if (m_line.contains(m_rxVariable, &rxMatch)) {
            m_line = rxMatch.captured(1);
            if (m_showAssignments->isChecked()) {
                // Assignment could be improved, e.g. catch strings from m_niceLine
                // but I'm not sure if variables are so important. I have kept
                // them only for "historic reasons"
                m_line.append(QLatin1Char(' ') + rxMatch.captured(2));
            }
            addNode(VariableNode, m_line, m_lineNumber);

        } else if (m_line.contains(m_rxFunction, &rxMatch)) {
            m_line = rxMatch.captured(1);
            if (m_showParameters->isChecked()) {
                m_line.append(QLatin1Char(' ') + rxMatch.captured(2));
            }
            addNode(FunctionNode, m_line, m_lineNumber);
        }
    }
}


bool TclParser::lineIsGood()
{
    // contiuation by backslash
    static const QRegularExpression rx1(QStringLiteral("[^\\\\]\\\\$"));
    if (m_line.contains(rx1)) {
        m_line.chop(1);
        return false;
    }

    // heredoc like continuation
    static const QRegularExpression rx2(QStringLiteral("[^\\\\]\""));
    bool oddQuoteNumbers = (m_line.count(rx2) % 2) == 1;
    bool heredoc = m_funcAtWork.contains(Me_At_Work); // Just for readability
    if (!oddQuoteNumbers && heredoc) {
        // heredoc = false;
        m_funcAtWork.remove(Me_At_Work);
    } else if (oddQuoteNumbers && !heredoc) {
        // heredoc = true;
        m_funcAtWork.insert(Me_At_Work);
        return false;
    } else if (heredoc) {
        return false;
    }

    return true;
}


void TclParser::removeStrings()
{
    // Remove "Bracket Commands", see testfile.tcl
    // Modification of ProgramParser::removeSingle/DoubleQuotedStrings()
    static const QRegularExpression rx(QStringLiteral("\\[[^\\[\\\\]*(?:\\\\.[^\\]\\\\]*)*\\]"));
    m_line.remove(rx);

    removeDoubleQuotedStrings();
}


void TclParser::removeComment()
{
    if (m_funcAtWork.contains(Me_At_Work)) {
        static const QRegularExpression rx(QStringLiteral("[^\\\\]\\\\$"));
        if (!m_line.contains(rx)) {
            m_funcAtWork.remove(Me_At_Work);
        }
        m_line.clear();
    }
    // Comment contiuation by backslash
    static const QRegularExpression rx(QStringLiteral("^#.*[^\\\\]\\\\$"));
    if (m_line.contains(rx)) {
        m_funcAtWork.insert(Me_At_Work);
        m_line.clear();
    }

    removeTclIf0Comment();
    // Remove "somehow" inline comment
    // NOTE Braces are treated as comment terminator, see testfile.tcl
    static const QRegularExpression rx2(QStringLiteral("#[^{}]*"));
    m_line.remove(rx2);
}


void TclParser::removeTclIf0Comment()
{
    static int myNestingLevel;

    if (m_funcAtWork.contains(Me_At_Work)) {
        checkForBlocks();
        checkNesting();
        if (nestingLevel() == myNestingLevel) {
            m_funcAtWork.remove(Me_At_Work);
        }
        m_line.clear();
    }

    static const QRegularExpression rx(QStringLiteral("^if\\s+0\\s*\\{"));
    if (m_line.contains(rx)) {
        myNestingLevel = nestingLevel();
        checkForBlocks();
        checkNesting();
        m_funcAtWork.insert(Me_At_Work);
        m_line.clear();
    }
}

// kate: space-indent on; indent-width 4; replace-tabs on;
