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


#ifndef INDEXVIEW_PERLPARSER_CLASS_H
#define INDEXVIEW_PERLPARSER_CLASS_H

#include "program_parser.h"

/**
 * The \p PerlParser provide rudimentary support for Perl files
 *
 * @author loh.tar
 */
class PerlParser : public ProgramParser
{
    Q_OBJECT

public:
    PerlParser(IndexView *view);
   ~PerlParser();

protected:
   enum NodeType {
        PragmaNode = FirstNodeType,
        UsesNode,
        VariableNode,
        SubroutineNode,
    };

    QString version() override { return QStringLiteral("0.5.1, Jul 2022"); } ;
    QString author() override { return QStringLiteral("2018 loh.tar \n\nInspired by \n2003 Massimo Callegari"); } ;

    void parseDocument() override;
    void removeStrings() override;
    void removeComment() override;
    void removePerlPod();

    QRegularExpression m_rxUses;
    QRegularExpression m_rxPragma;
    QRegularExpression m_rxVariable1;
    QRegularExpression m_rxVariable2;
    QRegularExpression m_rxSubroutine;

};

#endif

// kate: space-indent on; indent-width 4; replace-tabs on;
