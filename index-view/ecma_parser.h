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


#ifndef INDEXVIEW_ECMAPARSER_CLASS_H
#define INDEXVIEW_ECMAPARSER_CLASS_H

#include "program_parser.h"

/**
 * The \p EcmaParser provide rudimentary support for JavaSript/JScript, QML and
 * ActionScript 2.0 files
 *
 * @author loh.tar
 */
class EcmaParser : public ProgramParser
{
    Q_OBJECT

public:
    EcmaParser(QObject *view, KTextEditor::Document *doc);
   ~EcmaParser();

protected:
   enum NodeType {
        FunctionNode = FirstNodeType,
        Func2Node,
        Func3Node,
    };

    QString version() override { return QStringLiteral("0.5.1, Jul 2022"); } ;
    QString author() override { return QStringLiteral("2018 loh.tar \n\nInspired by \n2012 Jesse Crossen"); } ;

    void parseDocument() override;
    bool lineIsGood() override;
    void removeStrings() override;
    void removeComment() override;

    QRegularExpression m_rxFunction;
    QRegularExpression m_rxFunc2;
    QRegularExpression m_rxFunc3;
    QRegularExpression m_rxFunc4;

};

#endif

// kate: space-indent on; indent-width 4; replace-tabs on;
