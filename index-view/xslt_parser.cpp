
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


#include <KLocalizedString>

#include "icon_collection.h"
#include "index_view.h"

#include "xslt_parser.h"


XsltParser::XsltParser(IndexView *view)
    : ProgramParser(view)
{
    using namespace IconCollection;
    registerViewOption(ParamNode, SmallGreenIcon, QStringLiteral("Params"), i18n("Show Params"));
    registerViewOption(VariableNode, VariableIcon, QStringLiteral("Variables"), i18n("Show Variables"));
    registerViewOption(TemplateNode, RedBlueIcon, QStringLiteral("Templates"), i18n("Show Templates"));

    m_tagTypes.insert(ParamNode, QStringLiteral("xsl:param"));
    m_tagTypes.insert(VariableNode, QStringLiteral("xsl:variable"));
    m_tagTypes.insert(TemplateNode, QStringLiteral("xsl:template"));

    // Not sure if that's solution is best. Only one kind of TemplateNode uses "match"
    // and the orig parser choose then a different icon
    QString regExpMask = QStringLiteral("\\s%1=\"(.+)\"");
    m_attributes << QRegularExpression(regExpMask.arg(QStringLiteral("name")), QRegularExpression::InvertedGreedinessOption);
    m_attributes << QRegularExpression(regExpMask.arg(QStringLiteral("match")), QRegularExpression::InvertedGreedinessOption);
}


XsltParser::~XsltParser()
{
}


void XsltParser::parseDocument()
{
    QSet<QString> endTags;

    while (nextTag()) {
        // Test for known tag types
//         for (int nodeType = 0; m_tagTypes.contains(nodeType); nodeType++) {
        for (int nodeType = FirstNodeType; nodeType < LastNodeType; nodeType++) {
            QString tagType = m_tagTypes.value(nodeType);
            if (m_tag.startsWith(tagType)) {
                for (QRegularExpression rx : qAsConst(m_attributes)) {
                    if (m_tag.contains(rx)) {
                        addNode(nodeType, rx.match(m_tag).captured(1), m_lineNumber);
                        break;
                    }
                }

                // Do the nesting, or not...
                if (!m_tag.endsWith(QLatin1Char('/'))) {
                    beginOfBlock();
                    // ...but remember the needed end tag
                    tagType.prepend(QLatin1Char('/'));
                    endTags.insert(tagType);
                }
                continue;
            }
        }

        // Test for important end tags
        for (QString tagType : qAsConst(endTags)) {
            if (m_tag.startsWith(tagType)) {
                endOfBlock();
            }
        }
    }
}


bool XsltParser::lineIsGood()
{
    if (m_line.count(QLatin1Char('<')) - m_line.count(QLatin1Char('>')) != 0) {
        m_line.append(QLatin1Char(' '));
        return false;
    }

    return true;
}


bool XsltParser::nextTag()
{
    static QStringList tags;
    static int i = 0;

    if (i >= tags.size()) {
        nextInstruction();
        if (m_line.isEmpty()) {
            return false;
        }
        tags = m_line.split(QRegularExpression(QStringLiteral("[<>]")), Qt::SkipEmptyParts);
        i = 0;
// qDebug() << "NEW" << tags;
    }

    m_tag = tags.at(i++);

// qDebug() << ">>" << i << m_line << tags.size() << m_lineNumber;

    return true;
}


void XsltParser::removeStrings()
{
    // Well, here will not strings removed but tag content because this time
    // are not the strings dangerous but the tag content. Furthermore eases this
    // a lot the problem to extract the tag attributes
    m_line.replace(QRegularExpression(QStringLiteral(">.*<")), QStringLiteral("><"));
}


void XsltParser::removeComment()
{
    if (m_funcAtWork.contains(Me_At_Work)) {
        if (m_line.contains(QStringLiteral("-->"))) {
            m_line = m_line.section(QStringLiteral("-->"), 0, 0);
            m_funcAtWork.remove(Me_At_Work);
        } else {
            m_line.clear();
        }
    }
    if (m_line.contains(QStringLiteral("<!--"))) {
        m_line = m_line.section(QStringLiteral("<!--"), -1, -1);
        m_line = m_line.trimmed();
        m_funcAtWork.insert(Me_At_Work);
    }

    m_line = m_line.remove(QRegularExpression(QStringLiteral("<!--.*-->")));
}

// kate: space-indent on; indent-width 4; replace-tabs on;
