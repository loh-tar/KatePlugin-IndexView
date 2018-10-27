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


PerlParser::PerlParser(IndexView *view)
    : ProgramParser(view)
{
    using namespace IconCollection;
    registerViewOption(PragmaNode, GreenYellowIcon, QStringLiteral("Pragmas"), i18n("Show Pragmas"));
    registerViewOption(UsesNode, BlueIcon, QStringLiteral("Uses"), i18n("Show Uses"));
    registerViewOption(VariableNode, VariableIcon, QStringLiteral("Variables"), i18n("Show Variables"));
    registerViewOption(SubroutineNode, FunctionIcon, QStringLiteral("Subroutines"), i18n("Show Subroutines"));

    m_nonBlockElements << UsesNode << PragmaNode << VariableNode;

    m_rxUses = QRegExp(QStringLiteral("^use ([A-Z][\\w:]*)+(.*)?;"));
    m_rxPragma = QRegExp(QStringLiteral("^use ([a-z0-9].*);"));
    m_rxSubroutine = QRegExp(QStringLiteral("^sub (\\w+)"));
    m_rxVariable1 = QRegExp(QStringLiteral("\\bmy([\\@\\$\\%]\\w+)"));
    m_rxVariable2 = QRegExp(QStringLiteral("\\bmy\\((.*)\\)"));
//     m_rx = QRegExp(QStringLiteral(""));

    initHereDoc(QStringLiteral("<<~?"), QStringLiteral("\"'`"));
}


PerlParser::~PerlParser()
{
}


void PerlParser::parseDocument()
{
    while (nextInstruction()) {

        if (m_line.contains(m_rxUses)) {
            // http://perldoc.perl.org/functions/use.html
            addNode(UsesNode, m_rxUses.cap(1), m_lineNumber);

        } else if (m_line.contains(m_rxPragma)) {
            m_niceLine.contains(m_rxPragma);
            addNode(PragmaNode, m_rxPragma.cap(1), m_lineNumber);

        } else if (m_line.contains(m_rxSubroutine)) {
            addNode(SubroutineNode, m_rxSubroutine.cap(1), m_lineNumber);

        } else if (m_line.contains(m_rxVariable1)) {
            //http://perldoc.perl.org/functions/my.html
            addNode(VariableNode, m_rxVariable1.cap(1), m_lineNumber);

        } else if (m_line.contains(m_rxVariable2)) {
            addNode(VariableNode, m_rxVariable2.cap(1), m_lineNumber);

        }
    }
}


void PerlParser::removeStrings()
{
    removeSingleQuotedStrings();
    removeDoubleQuotedStrings();
    // Special Perl quoting
    // FIXME These are too simple, adopt ProgramParser::removeSingle/DoubleQuotedStrings()
    m_line.remove(QRegExp(QStringLiteral("\\q\\{.*\\}")));
    m_line.remove(QRegExp(QStringLiteral("\\q\\#.*\\#")));
    m_line.remove(QRegExp(QStringLiteral("\\q\\^.*\\^")));
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
    if (m_funcAtWork.contains(Me_At_Work)) {
        // Skip Perl's special documentation block
        if (m_line.contains(QRegExp(QStringLiteral("^=cut$")))) {
            m_funcAtWork.remove(Me_At_Work);
        }
        m_line.clear();

    } else if (m_line.contains(QRegExp(QStringLiteral("^=[a-zA-Z]")))) {
        m_funcAtWork.insert(Me_At_Work);
        m_line.clear();
    }
}

// kate: space-indent on; indent-width 4; replace-tabs on;
