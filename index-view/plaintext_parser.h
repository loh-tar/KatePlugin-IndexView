/*   This file is part of KatePlugin-IndexView
 *
 *   PlainTextParser Class
 *   Copyright (C) 2018 loh.tar@googlemail.com
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


#ifndef INDEXVIEW_PLAINTEXTPARSER_CLASS_H
#define INDEXVIEW_PLAINTEXTPARSER_CLASS_H

#include "document_parser.h"

/**
 * The \p PlainTextParser provide something like a table of contents
 *
 * @author loh.tar
 */
class PlainTextParser : public DocumentParser
{
    Q_OBJECT

public:
    PlainTextParser(IndexView *view);
   ~PlainTextParser();

protected:
   enum NodeType {
        RootNode,
        SectNode,
        HeadNode,
        ParaNode
    };

    enum LineType {
        EmptyLine,
        NormalLine,
        DashLine,
        EqualLine
    };

    QString version() override { return QStringLiteral("0.8, Okt 2018"); } ;
    QString author() override { return QStringLiteral("2018 loh.tar"); } ;

    void parseDocument() override;

};

#endif

// kate: space-indent on; indent-width 4; replace-tabs on;
