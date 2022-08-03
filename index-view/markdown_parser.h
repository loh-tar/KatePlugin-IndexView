/*   This file is part of KatePlugin-IndexView
 *
 *   MarkdownParser Class
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


#ifndef INDEXVIEW_MARKDOWNPARSER_CLASS_H
#define INDEXVIEW_MARKDOWNPARSER_CLASS_H

#include "plaintext_parser.h"

/**
 * The \p MarkdownParser provide something like a table of contents.
 * You will find more information about supported, or not (yet) supported,
 * features in testfile.md.
 *
 * @author loh.tar
 */
class MarkdownParser : public DocumentParser
{
    Q_OBJECT

public:
    MarkdownParser(IndexView *view, const QString &docType);
   ~MarkdownParser();

protected:
    enum NodeType {
        Head1Node = FirstCustomNodeType,
        Head2Node,
        Head3Node,
        Head4Node,
        Head5Node,
        Head6Node,
        ParaNode
    };

    enum LineType {
        NormalLine = FirstCustomLineType,
        HeaderLine,
        EqualLine,
        DashLine
    };

    QString version() override { return QStringLiteral("0.7.3 Aug 2022"); } ;
    QString author() override { return QStringLiteral("2018, 2022 loh.tar"); } ;

    void parseDocument() override;

};

/**
 * The \p AsciiDocParser provide something like a table of contents.
 * You will find more information about supported, or not (yet) supported,
 * features in testfile.adoc.
 *
 * @author loh.tar
 */
class AsciiDocParser : public DocumentParser
{
    Q_OBJECT

public:
    AsciiDocParser(IndexView *view, const QString &docType);
   ~AsciiDocParser();

protected:
    enum NodeType {
        Head1Node = FirstCustomNodeType,
        Head2Node,
        Head3Node,
        Head4Node,
        Head5Node,
        Head6Node,
        ParaNode
    };

    enum LineType {
        NormalLine = FirstCustomLineType,
        HeaderLine,
        EqualLine,
        DashLine
    };

    QString version() override { return QStringLiteral("0.5 Aug 2022"); } ;
    QString author() override { return QStringLiteral("2022 loh.tar"); } ;

    void parseDocument() override;

};

#endif

// kate: space-indent on; indent-width 4; replace-tabs on;
