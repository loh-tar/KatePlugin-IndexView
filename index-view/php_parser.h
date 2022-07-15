/*   This file is part of KatePlugin-IndexView
 *
 *   PhpParser Class
 *   Copyright (C) 2018 loh.tar@googlemail.com
 *
 *   Inspired by php_parser.cpp, part of Kate's SymbolViewer
 *   Copyright (C) 2010 Emmanuel Bouthenot <kolter@openics.org>
 *   Copyright (C) 2007 Massimo Callegari <massimocallegari@yahoo.it>
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


#ifndef INDEXVIEW_PHPPARSER_CLASS_H
#define INDEXVIEW_PHPPARSER_CLASS_H

#include "program_parser.h"

/**
 * The \p PhpParser provide support for PHP files
 *
 * @author loh.tar
 */
class PhpParser : public ProgramParser
{
    Q_OBJECT

public:
    PhpParser(IndexView *view);
   ~PhpParser();

protected:
   enum NodeType {
        NamespaceNode = FirstNodeType,
        InterfaceNode,
        ClassNode,
        FunctionNode,
        ConstantNode,
        VariableNode,
    };

    QString version() override { return QStringLiteral("0.8.1, Jul 2022"); } ;
    QString author() override { return QStringLiteral("2018 loh.tar \n\nInspired by \n"
                                                      "2010 Emmanuel Bouthenot \n"
                                                      "2007 Massimo Callegari"); } ;

    void parseDocument() override;
    void removeStrings() override;
    void removeComment() override;

    QRegularExpression m_rxDefine;
    QRegularExpression m_rxConst;
    QRegularExpression m_rxNamespace;
    QRegularExpression m_rxClass;
    QRegularExpression m_rxInterface;
    QRegularExpression m_rxFunction;
    QRegularExpression m_rxVariable;

};

#endif

// kate: space-indent on; indent-width 4; replace-tabs on;
