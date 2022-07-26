/*   This file is part of KatePlugin-IndexView
 *
 *   PerlParser Class
 *   Copyright (C) 2018 loh.tar@googlemail.com
 *
 *   Inspired by perl_parser.cpp, part of Kate's SymbolViewer
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

#include "perl_parser.h"


PerlParser::PerlParser(IndexView *view, const QString &docType)
    : ProgramParser(view, docType)
{
    using namespace IconCollection;
    registerViewOption(UsesNode, UsesIcon, QStringLiteral("Uses"), i18n("Show Uses"));
    registerViewOption(PragmaNode, PragmaIcon, QStringLiteral("Pragmas"), i18n("Show Pragmas"));
    registerViewOption(VariableNode, VariableIcon, QStringLiteral("Variables"), i18n("Show Variables"));
    registerViewOption(SubroutineNode, FuncDefIcon, QStringLiteral("Subroutines"), i18n("Show Subroutines"));

    m_nonBlockElements << UsesNode << PragmaNode << VariableNode;

    m_rxUses = QRegularExpression(QStringLiteral("^use ([A-Z][\\w:]*)+(.*)?;"));
    m_rxPragma = QRegularExpression(QStringLiteral("^use ([a-z0-9].*);"));
    m_rxSubroutine = QRegularExpression(QStringLiteral("^sub (\\w+)"));
    m_rxVariable1 = QRegularExpression(QStringLiteral("\\bmy([\\@\\$\\%]\\w+)"));
    m_rxVariable2 = QRegularExpression(QStringLiteral("\\bmy\\((.*)\\)"));
//     m_rx = QRegularExpression(QStringLiteral(""));

    initHereDoc(QStringLiteral("<<~?"), QStringLiteral("\"'`"));
}


PerlParser::~PerlParser()
{
}


void PerlParser::parseDocument()
{
    QRegularExpressionMatch rxMatch;

    while (nextInstruction()) {

        if (m_line.contains(m_rxUses, &rxMatch)) {
            // http://perldoc.perl.org/functions/use.html
            addNode(UsesNode, rxMatch.captured(1), m_lineNumber);

        } else if (m_line.contains(m_rxPragma, &rxMatch)) {
            m_niceLine.contains(m_rxPragma);
            addNode(PragmaNode, rxMatch.captured(1), m_lineNumber);

        } else if (m_line.contains(m_rxSubroutine, &rxMatch)) {
            addNode(SubroutineNode, rxMatch.captured(1), m_lineNumber);

        } else if (m_line.contains(m_rxVariable1, &rxMatch)) {
            //http://perldoc.perl.org/functions/my.html
            addNode(VariableNode, rxMatch.captured(1), m_lineNumber);

        } else if (m_line.contains(m_rxVariable2, &rxMatch)) {
            addNode(VariableNode, rxMatch.captured(1), m_lineNumber);

        }
    }
}


void PerlParser::removeStrings()
{
    removeSingleQuotedStrings();
    removeDoubleQuotedStrings();
    // Special Perl quoting
    // FIXME These are too simple, adopt ProgramParser::removeSingle/DoubleQuotedStrings()
    static const QRegularExpression rx1(QStringLiteral("q\\{.*\\}"));
    m_line.remove(rx1);
    static const QRegularExpression rx2(QStringLiteral("q\\#.*\\#"));
    m_line.remove(rx2);
    static const QRegularExpression rx3(QStringLiteral("q\\^.*\\^"));
    m_line.remove(rx3);
}


void PerlParser::removeComment()
{
    removeTrailingSharpComment();
    removePerlPod();
    removeHereDoc();
}


void PerlParser::removePerlPod()
{
    // http://perldoc.perl.org/perlpod.html
    // Skip Perl's special documentation block
    static const QRegularExpression rx1(QStringLiteral("^=cut$"));
    static const QRegularExpression rx2(QStringLiteral("^=[a-zA-Z]"));

    if (m_funcAtWork.contains(Me_At_Work)) {
        if (m_line.contains(rx1)) {
            m_funcAtWork.remove(Me_At_Work);
        }
        m_line.clear();

    } else if (m_line.contains(rx2)) {
        m_funcAtWork.insert(Me_At_Work);
        m_line.clear();
    }
}

// kate: space-indent on; indent-width 4; replace-tabs on;
