/*   This file is part of KatePlugin-IndexView
 *
 *   EcmaParser Class
 *   Copyright (C) 2018 loh.tar@googlemail.com
 *
 *   Inspired by ecma_parser.cpp, part of Kate's SymbolViewer
 *   Copyright (C) 2012 Jesse Crossen <jesse.crossen@gmail.com>
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

#include "ecma_parser.h"


EcmaParser::EcmaParser(QObject *view, KTextEditor::Document *doc)
    : ProgramParser(view, doc)
{
    using namespace IconCollection;
    // FIXME Give all these "Functions" more fitting names
    // See https://wiki.selfhtml.org/wiki/JavaScript/Objekte_-_Eigenschaften_und_Methoden
    registerViewOption(FunctionNode, FuncDefIcon, QStringLiteral("Functions"), i18n("Show Functions"));
    registerViewOption(Func2Node, Cyan2Icon, QStringLiteral("Func2"), i18n("Show Func2"));
    registerViewOption(Func3Node, Blue2Icon, QStringLiteral("Func3"), i18n("Show Func3"));

    m_rxFunction = QRegularExpression(QStringLiteral("\\bfunction (\\w+)\\("));
    m_rxFunc2 = QRegularExpression(QStringLiteral("\\b(\\w+)=function\\((.*)\\)"));
    m_rxFunc3 = QRegularExpression(QStringLiteral("\\b(\\w+):function\\((.*)\\)"));
    // Must match against m_niceLine, so consider spaces
    m_rxFunc4 = QRegularExpression(QStringLiteral("\\[\'(\\w+)\'\\]\\s?=\\s?function\\((.*)\\)"));
}


EcmaParser::~EcmaParser()
{
}


void EcmaParser::parseDocument()
{
    QRegularExpressionMatch rxMatch;

    while (nextInstruction()) {

        if (m_line.contains(m_rxFunction, &rxMatch)) {
            addNode(FunctionNode, rxMatch.captured(1), m_lineNumber);

        } else if (m_line.contains(m_rxFunc2, &rxMatch)) {
            addNode(Func2Node, rxMatch.captured(1), m_lineNumber);

        } else if (m_line.contains(m_rxFunc3, &rxMatch)) {
            addNode(Func3Node, rxMatch.captured(1), m_lineNumber);

        } else if (m_niceLine.contains(m_rxFunc4, &rxMatch)) {
            addNode(Func2Node, rxMatch.captured(1), m_lineNumber);

        }
    }
}


bool EcmaParser::lineIsGood()
{
    // >The five restricted productions are return, throw, break, continue, and post-increment/decrement.
    static const QRegularExpression rx1(QStringLiteral("\\b(return|throw|break|continue)\\b(.*)?$"));
    static const QRegularExpression rx2(QStringLiteral("(\\+\\+|--)$"));

    if (ProgramParser::lineIsGood()) {
        return true;

    // https://en.wikipedia.org/wiki/JavaScript_syntax#Whitespace_and_semicolons
    // > Some suggest instead the use of leading semicolons ...This is known as a defensive semicolon,
    // > and is particularly recommended
    } else if (rawLine(1).startsWith(QLatin1Char(';'))) {
        return true;

    } else if (m_line.contains(rx1)) {
        return true;
    } else if (m_line.contains(rx2)) {
        return true;

    // Due to our test file, from https://en.wikipedia.org/wiki/JavaScript
    } else if (m_line.endsWith(QStringLiteral("},"))) {
        return true;

    }

    return false;
}


void EcmaParser::removeStrings()
{
    removeSingleQuotedStrings();
    removeDoubleQuotedStrings();
}


void EcmaParser::removeComment()
{
    removeMultiLineSlashStarComment();
    removeTrailingDoubleSlashComment();
}

// kate: space-indent on; indent-width 4; replace-tabs on;
