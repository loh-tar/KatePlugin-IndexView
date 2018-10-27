/*   This file is part of KatePlugin-IndexView
 *
 *   RubyParser Class
 *   Copyright (C) 2018 loh.tar@googlemail.com
 *
 *   Inspired by ruby_parser.cpp, part of Kate's SymbolViewer
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


#ifndef INDEXVIEW_RUBYPARSER_CLASS_H
#define INDEXVIEW_RUBYPARSER_CLASS_H

#include "program_parser.h"

/**
 * The \p RubyParser provide support for Ruby files
 *
 * @author loh.tar
 */
class RubyParser : public ProgramParser
{
    Q_OBJECT

public:
    RubyParser(IndexView *view);
   ~RubyParser();

protected:
   enum NodeType {
        MethodNode = FirstNodeType,
        ModuleNode,
        ClassNode,
        MixinNode
    };

    QString version() override { return QStringLiteral("0.7, Okt 2018"); } ;
    QString author() override { return QStringLiteral("2018 loh.tar \n\nInspired by \n2007 Massimo Callegari"); } ;

    void parseDocument() override;
    bool lineIsGood() override { return true; };

    QAction     *m_showParameters;
};

#endif

// kate: space-indent on; indent-width 4; replace-tabs on;
