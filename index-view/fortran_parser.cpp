/*   This file is part of KatePlugin-IndexView
 *
 *   FortranParser Class
 *   Copyright (C) 2018 loh.tar@googlemail.com
 *
 *   Inspired by fortran_parser.cpp, part of Kate's SymbolViewer
 *   Copyright (C) 2005 Roberto Quitiliani <roby(dot)q(AT)tiscali(dot)it>
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

#include "fortran_parser.h"


FortranParser::FortranParser(IndexView *view)
    : ProgramParser(view)
{
    using namespace IconCollection;
    registerViewOption(TypeNode, TypeIcon, QStringLiteral("Types"), i18n("Show Types"));
    registerViewOption(FunctionNode, FuncDefIcon, QStringLiteral("Functions"), i18n("Show Functions"));
    registerViewOption(SubroutineNode, SubroutineIcon, QStringLiteral("Subroutines"), i18n("Show Subroutines"));
    registerViewOption(ModuleNode, Green2Icon, QStringLiteral("Modules"), i18n("Show Modules"));

    // See Ruby
//  m_blockElements << QStringLiteral("if") << QStringLiteral("do") << QStringLiteral("for") << QStringLiteral("select")
//     << QStringLiteral("interface")
//     << QStringLiteral("where")
//     << QStringLiteral("")

    m_rxFunction   = QRegularExpression(QStringLiteral("\\bfunction (\\w+)\\(([\\w\\s,]*)?\\)"), QRegularExpression::CaseInsensitiveOption);
    m_rxSubroutine = QRegularExpression(QStringLiteral("\\bsubroutine (\\w+)\\(([\\w\\s,]*)?\\)"), QRegularExpression::CaseInsensitiveOption);
    m_rxModule     = QRegularExpression(QStringLiteral("^module (\\w+)"), QRegularExpression::CaseInsensitiveOption);
    m_rxType       = QRegularExpression(QStringLiteral("^type (\\w+)"), QRegularExpression::CaseInsensitiveOption);
    m_rxEnd        = QRegularExpression(QStringLiteral("^end (\\w+)"), QRegularExpression::CaseInsensitiveOption);
    m_rxProgram    = QRegularExpression(QStringLiteral("^program (\\w+)"), QRegularExpression::CaseInsensitiveOption);
}


FortranParser::~FortranParser()
{
}


void FortranParser::parseDocument()
{
    m_BlockStack.clear();
    QRegularExpressionMatch rxMatch;

    while (nextInstruction()) {
        //Fortran is case insensitive
        m_line = m_line.toLower();

        if (m_line.contains(m_rxEnd, &rxMatch)) {
            endBlock(m_rxEnd.match(m_line).captured(1));

        } else if (m_line.contains(m_rxSubroutine, &rxMatch)) {
            addNode(SubroutineNode, rxMatch.captured(1), m_lineNumber);
            beginBlock(QStringLiteral("subroutine"));

        } else if (m_line.startsWith(QStringLiteral("module procedure "))) {
            // Ignored
        } else if (m_line.contains(m_rxModule, &rxMatch)) {
            addNode(ModuleNode, rxMatch.captured(1), m_lineNumber);
            beginBlock(QStringLiteral("module"));

        } else if (m_line.contains(m_rxType, &rxMatch)) {
            addNode(TypeNode, rxMatch.captured(1), m_lineNumber);
            beginBlock(QStringLiteral("type"));

        } else if (m_line.contains(m_rxFunction, &rxMatch)) {
            addNode(FunctionNode, rxMatch.captured(1), m_lineNumber);
            beginBlock(QStringLiteral("function"));

        } else if (m_line.contains(m_rxProgram, &rxMatch)) {
            addNode(SubroutineNode, QStringLiteral("Main: ") + rxMatch.captured(1), m_lineNumber);
            beginBlock(QStringLiteral("program"));

        }
    }
}


void FortranParser::beginBlock(const QString  &type)
{
    m_BlockStack.push(type.toLower());
    beginOfBlock();
}


void FortranParser::endBlock(const QString  &type)
{
    if (m_BlockStack.isEmpty()) {
        return;
    }

    if (m_BlockStack.top() == type.toLower()) {
        m_BlockStack.pop();
        endOfBlock();
    }
}


void FortranParser::removeStrings()
{
    removeSingleQuotedStrings();
    removeDoubleQuotedStrings();
}


void FortranParser::removeComment()
{
    if (m_line.isEmpty()) {
        return;
    }

    // Yes, two times RAW-line and once line
    // https://en.wikipedia.org/wiki/Fortran#Obsolescence_and_deletions
    // Says: Column 1 contains C or * or ! for comments.
    if (   rawLine().at(0) == QLatin1Char('c')
        || rawLine().at(0) == QLatin1Char('*')
        ||    m_line.at(0) == QLatin1Char('!')) {

        m_line.clear();
    }
}

// kate: space-indent on; indent-width 4; replace-tabs on;
