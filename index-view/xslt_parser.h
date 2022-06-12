/*   This file is part of KatePlugin-IndexView
 *
 *   XsltParser Class
 *   Copyright (C) 2018 loh.tar@googlemail.com
 *
 *   Inspired by xslt_parser.cpp, part of Kate's SymbolViewer
 *   Copyright (C) 2007 Jiri Tyr <jiri.tyr@vslib.cz>
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


#ifndef INDEXVIEW_XSLTPARSER_CLASS_H
#define INDEXVIEW_XSLTPARSER_CLASS_H

#include "program_parser.h"

/**
 * The \p XsltParser provide rudimentary support for Xslt files
 *
 * @author loh.tar
 */

// Parsing XML is a welcome event for sarcastic comments
// https://stackoverflow.com/q/1732348
//
// However, I think the current solution works good enough for our purpose and
// has the potential for a XmlParser master class.
//
// But feel free to make a XmlParser master class based on Qt's XML processing
// http://doc.qt.io/qt-5/xml-processing.html

class XsltParser : public ProgramParser
{
    Q_OBJECT

public:
    XsltParser(IndexView *view);
   ~XsltParser();

protected:
   enum NodeType {
        ParamNode = FirstNodeType,
        VariableNode,
        TemplateNode,
        LastNodeType
    };

    QString version() override { return QStringLiteral("0.5, Okt 2018"); } ;
    QString author() override { return QStringLiteral("2018 loh.tar \n\nInspired by \n2007 Jiri Tyr"); } ;

    void parseDocument() override;

    /**
     * Uses appendNextLine() to deliver each tag on @c m_line in @c m_tag with
     * removed angle brackets.
     */
    bool nextTag();

    bool lineIsGood() override;
    void removeStrings() override;
    void removeComment() override;
    int  checkForBlocks() override { return 0; } // Not needed

    QHash<int, QString>         m_tagTypes;
    QList<QRegularExpression>   m_attributes;
    QString                     m_tag;

};

#endif

// kate: space-indent on; indent-width 4; replace-tabs on;
