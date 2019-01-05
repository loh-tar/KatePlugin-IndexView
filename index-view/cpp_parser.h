/*   This file is part of KatePlugin-IndexView
 *
 *   CppParser Class
 *   Copyright (C) 2018 loh.tar@googlemail.com
 *
 *   Inspired by cpp_parser.cpp, part of Kate's SymbolViewer
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


#ifndef INDEXVIEW_CPPPARSER_CLASS_H
#define INDEXVIEW_CPPPARSER_CLASS_H

#include "program_parser.h"

/**
 * The \p CppParser provide support for C++ files.
 *
 * @author loh.tar
 */
class CppParser : public ProgramParser
{
    Q_OBJECT

public:
    CppParser(IndexView *view);
   ~CppParser();

protected:
   enum NodeType {
        MacroNode = FirstNodeType,
        NamespaceNode,
        TypedefNode,
//         VariableNode,
        StructNode,
        FunctionDefNode,// Function definition
        FunctionDecNode,// Function declaration
    };

    QString version() override { return QStringLiteral("0.8.2, Jan 2019"); } ;
    QString author() override { return QStringLiteral("2018 loh.tar \n\nInspired by \n2003 Massimo Callegari"); } ;

    void parseDocument() override;
    bool lineIsGood() override;
    bool appendNextLine() override;
    void removeStrings() override;
    void removeComment() override;

//     QAction     *m_showParameters; // FIXME if you really need need it

    QRegExp     m_rxStruct;
    QRegExp     m_rxEnum;
    QRegExp     m_rxFuncDef;
    QRegExp     m_rxFuncDec;
    QRegExp     m_rxTypedef;
    QRegExp     m_rxMarcro;
    QRegExp     m_rxNamespace;

    QSet<QString>   m_keywordsToIgnore; // To avoid false detection, e.g. as function
};

#endif

// kate: space-indent on; indent-width 4; replace-tabs on;
