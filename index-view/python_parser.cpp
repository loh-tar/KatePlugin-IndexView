/*   This file is part of KatePlugin-IndexView
 *
 *   PythonParser Class
 *   Copyright (C) 2018 loh.tar@googlemail.com
 *
 *   Inspired by python_parser.cpp, part of Kate's SymbolViewer
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

#include "python_parser.h"


PythonParser::PythonParser(QObject *view, const QString &docType, KTextEditor::Document *doc)
    : ProgramParser(view, docType, doc)
{
    using namespace IconCollection;
    registerViewOption(ClassNode, ClassIcon, QStringLiteral("Classes"), i18n("Show Classes"));
    registerViewOption(MethodNode, MethodIcon, QStringLiteral("Methods"), i18n("Show Methods"));
    registerViewOption(FunctionNode, FuncDefIcon, QStringLiteral("Functions"), i18n("Show Functions"));

    addViewOptionDependency(MethodNode, ClassNode);
}


PythonParser::~PythonParser()
{
}


void PythonParser::parseDocument()
{
    static const QRegularExpression rxNonSpace(QStringLiteral("\\S"));

    int lastIndent = 0;
    int currIndent = 0;

    while (nextInstruction()) {
        currIndent = rawLine().indexOf(rxNonSpace);
        if (currIndent < 1) {
            clearNesting();
        } else if (lastIndent < currIndent) {
            beginOfBlock();
        } else if (lastIndent > currIndent) {
            endOfBlock();
        }
        lastIndent = currIndent;

        // TODO Use regex/match too
        if (m_line.startsWith(QStringLiteral("class "))) {
            m_line = m_line.mid(6);
            m_line = m_line.section(QLatin1Char('('), 0, 0);
            addNode(ClassNode, m_line, m_lineNumber);

        } else if (m_line.startsWith(QStringLiteral("def "))) {
            m_line = m_line.mid(4);
            m_line = m_line.section(QLatin1Char('('), 0, 0);
            if (currIndent > 0) {
                addNode(MethodNode, m_line, m_lineNumber);
            } else {
                addNode(FunctionNode, m_line, m_lineNumber);
            }
        }
    }
}


void PythonParser::removeStrings()
{
    // Because removing strings would smash triple quotes we replace these by a placeholder
    // FIXME Better token idea? K-ate P-lugin I-ndex V-iew T-ripple D-ouble Q-uote
    static const QRegularExpression rx1(QStringLiteral("\"\"\""));
    m_line.replace(rx1, QStringLiteral("!KPIVTDQ!"));
    //                          K-ate P-lugin I-ndex V-iew T-ripple S-ingle Q-uote
    static const QRegularExpression rx2(QStringLiteral("\'\'\'"));
    m_line.replace(rx2, QStringLiteral("!KPIVTSQ!"));

    removeSingleQuotedStrings();
    removeDoubleQuotedStrings();
}


void PythonParser::removeComment()
{
    // FIXME Is it possible to split this and use a part in/as lineIsGood()?
    static QString quoteToken;

    if (m_funcAtWork.contains(Me_At_Work)) {
        // Remove block with triple quotes,
        // doing it dumb and eager because of to much unknown special cases
        if (m_line.contains(quoteToken)) {
            m_funcAtWork.remove(Me_At_Work);
        }
        m_line.clear();
        return;
    }

    removeTrailingSharpComment();

    // Remove single line with both ends of triple quotes
    static const QRegularExpression rx1(QStringLiteral("^!KPIVTDQ!.+!KPIVTDQ!$"));
    if (m_line.contains(rx1)) {
        m_line.clear();
        return;
    }
    static const QRegularExpression rx2(QStringLiteral("^!KPIVTSQ!.+!KPIVTSQ!$"));
    if (m_line.contains(rx2)) {
        m_line.clear();
        return;
    }

    // Check for triple quotes, first come, first serve
    int posDoubleQuotes = m_line.indexOf(QStringLiteral("!KPIVTDQ!")) + 1;
    int posSingleQuotes = m_line.indexOf(QStringLiteral("!KPIVTSQ!")) + 1;
    // We are only interested which one comes first, so we had add 1 for ease this test
    if (!posDoubleQuotes && !posSingleQuotes) {
        return;
    }

    if (!posDoubleQuotes) {
        quoteToken = QStringLiteral("!KPIVTSQ!");
    } else {
        quoteToken = QStringLiteral("!KPIVTDQ!");
    }

    m_funcAtWork.insert(Me_At_Work);
    m_line.clear();
}

// kate: space-indent on; indent-width 4; replace-tabs on;
