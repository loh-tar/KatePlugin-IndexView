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


#ifndef INDEXVIEW_TCLPARSER_CLASS_H
#define INDEXVIEW_TCLPARSER_CLASS_H

#include "program_parser.h"

/**
 * The \p TclParser provide rudimentary support for TCL files
 *
 * @author loh.tar
 */
class TclParser : public ProgramParser
{
    Q_OBJECT

public:
    TclParser(IndexView *view);
   ~TclParser();

protected:
   enum NodeType {
        VariableNode = FirstNodeType,
        FunctionNode,
    };

    QString version() override { return QStringLiteral("0.6.1, Jul 2022"); } ;
    QString author() override { return QStringLiteral("2018 loh.tar \n\nInspired by \n2003 Massimo Callegari"); } ;

    void parseDocument() override;
    bool lineIsGood() override;
    void removeStrings() override;
    void removeComment() override;
    void removeTclIf0Comment();

    QAction            *m_showAssignments;
    QAction            *m_showParameters;

    QRegularExpression  m_rxVariable;
    QRegularExpression  m_rxFunction;
};

#endif

// kate: space-indent on; indent-width 4; replace-tabs on;
