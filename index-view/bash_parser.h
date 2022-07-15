/*   This file is part of KatePlugin-IndexView
 *
 *   BashParser Class
 *   Copyright (C) 2018 loh.tar@googlemail.com
 *
 *   Inspired by bash_parser.cpp, part of Kate's SymbolViewer
 *   Copyright (C) 2008 Daniel Dumitrache <daniel.dumitrache@gmail.com>
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

#ifndef INDEXVIEW_BASHPARSER_CLASS_H
#define INDEXVIEW_BASHPARSER_CLASS_H

#include "program_parser.h"

/**
 * The \p BashParser provide rudimentary support for Bash files
 *
 * @author loh.tar
 */
class BashParser : public ProgramParser
{
    Q_OBJECT

public:
    BashParser(IndexView *view);
   ~BashParser();

protected:
   enum NodeType {
        FunctionNode = FirstNodeType,
    };

    QString version() override { return QStringLiteral("0.5.1, Jul 2022"); } ;
    QString author() override { return QStringLiteral("2018 loh.tar \n\nInspired by \n2008 Daniel Dumitrache"); } ;

    void parseDocument() override;
    bool lineIsGood() override { return true; };
    void removeComment() override;

};

#endif

// kate: space-indent on; indent-width 4; replace-tabs on;
