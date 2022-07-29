/*   This file is part of KatePlugin-IndexView
 *
 *   PythonParser Class
 *   Copyright (C) 2018 loh.tar@googlemail.com
 *
 *   Inspired by ruby_parser.cpp, part of Kate's SymbolViewer
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


#ifndef INDEXVIEW_PYTHONPARSER_CLASS_H
#define INDEXVIEW_PYTHONPARSER_CLASS_H

#include "program_parser.h"

/**
 * The \p PythonParser provide support for Python files
 *
 * @author loh.tar
 */
class PythonParser : public ProgramParser
{
    Q_OBJECT

public:
    PythonParser(IndexView *view, const QString &docType);
   ~PythonParser();

protected:
   enum NodeType {
        ClassNode = FirstNodeType,
        MethodNode,
        FunctionNode,
    };

    QString version() override { return QStringLiteral("0.7, Okt 2018"); } ;
    QString author() override { return QStringLiteral("2018 loh.tar \n\nInspired by \n2003 Massimo Callegari"); } ;

    void parseDocument() override;
    bool lineIsGood() override { return true; };
    void removeStrings() override;
    void removeComment() override;

};

#endif

// kate: space-indent on; indent-width 4; replace-tabs on;
