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
    PlainTextParser(IndexView *view, const QString &docType);
   ~PlainTextParser();

protected:
   enum NodeType {
        SectNode = FirstCustomNodeType,
        HeadNode,
        ParaNode
    };

    enum LineType {
        NormalLine = FirstCustomLineType,
        DashLine,
        EqualLine
    };

    QString version() override { return QStringLiteral("0.9, Jul 2022"); } ;
    QString author() override { return QStringLiteral("2018, 2022 loh.tar"); } ;

    void parseDocument() override;

};

/**
 * The \p IniFileParser provide a table of contents
 *
 * @author loh.tar
 */
class IniFileParser : public DocumentParser
{
    Q_OBJECT

public:
    IniFileParser(IndexView *view, const QString &docType);
    ~IniFileParser();

protected:
    enum NodeType {
        Section1Node = FirstCustomNodeType,
        Section2Node,
        Section3Node,
        Section4Node,
        Section5Node,
        Section6Node,
        LastNodeType  // Keep it at the end
    };

    QString version() override { return QStringLiteral("0.7, Jul 2022"); } ;
    QString author() override { return QStringLiteral("2022 loh.tar"); } ;

    void parseDocument() override;

    void addNodesToParent(QTreeWidgetItem *parentNode, int pos);
    QTreeWidgetItem *addNodeToParent(int nodeType, QTreeWidgetItem *parentNode, const QString &text);

    QStringList m_sections; // Collect all sub sections, like a path
};

/**
 * The \p DiffFileParser provide a table of contents
 *
 * @author loh.tar
 */
class DiffFileParser : public DocumentParser
{
    Q_OBJECT

public:
    DiffFileParser(IndexView *view, const QString &docType);
    ~DiffFileParser();

protected:
    enum NodeType {
        Section1Node = FirstCustomNodeType,
        Section2Node,
        Section3Node,
        Section4Node,
        Section5Node,
        Section6Node,
        ChunkNode,
        LastNodeType  // Keep it at the end
    };

    QString version() override { return QStringLiteral("0.5, Jul 2022"); } ;
    QString author() override { return QStringLiteral("2022 loh.tar\n\nATM is only the normal git diff format supported"); } ;

    void parseDocument() override;

    QAction *m_noNumberAsChunk;
};

#endif

// kate: space-indent on; indent-width 4; replace-tabs on;
