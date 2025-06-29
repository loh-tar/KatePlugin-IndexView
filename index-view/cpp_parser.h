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
    CppParser(QObject *view, KTextEditor::Document *doc);
   ~CppParser();

protected:
   enum NodeType {
        MacroNode = FirstNodeType,
        NamespaceNode,
        TypedefNode,
        StructNode,
        AccessSpecNode,
        EnumNode,
        FunctionDefNode,// Function definition
        FunctionDecNode,// Function declaration
        VariableNode,
    };

    QString version() override { return QStringLiteral("0.9.2, Jul 2025"); } ;
    QString author() override { return QStringLiteral("2018, 2022, 2025 loh.tar \n\nInspired by \n2003 Massimo Callegari"); } ;

    void parseDocument() override;
    bool lineIsGood() override;
    bool appendNextLine() override;
    void removeStrings() override;
    void removeComment() override;

    void addAccessSpecNode(const QString &accessSpec);
    void addFuncDefNode(const QString &retType, const QString &nameSpace, const QString &funcName);

    QAction            *m_showAccessSpec;
    //     QAction     *m_showParameters; // FIXME if you really need need it


    QSet<QString>       m_keywordsToIgnore; // To avoid false detection, e.g. as function
};

#endif

// kate: space-indent on; indent-width 4; replace-tabs on;
