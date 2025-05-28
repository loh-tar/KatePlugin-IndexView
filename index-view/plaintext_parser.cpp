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


#include <KLocalizedString>

#include "icon_collection.h"
#include "index_view.h"

#include "plaintext_parser.h"


PlainTextParser::PlainTextParser(QObject *view, KTextEditor::Document *doc)
    : DocumentParser(view, doc)
{
    using namespace IconCollection;
    registerViewOption(ParaNode, ParaIcon, QStringLiteral("Paragraph"), i18n("Show Paragraphs"));

    setNodeTypeIcon(SectNode, Head1Icon);
    setNodeTypeIcon(HeadNode, Head2Icon);
}


PlainTextParser::~PlainTextParser()
{
}


void PlainTextParser::parseDocument()
{
    static const QRegularExpression rxEqual(QStringLiteral("^[=#*]{3,}$"));
    static const QRegularExpression rxDash(QStringLiteral("^[-~^]{3,}$"));
    static const QRegularExpression rxIsoDate(QStringLiteral("^\\d{4}-([0]\\d|1[0-2])-([0-2]\\d|3[01])$"));

    QString paraLine;   // First line of a paragraph

    initHistory(3);

    while (nextLine()) {
        // Let's start the investigation
        bool currIsEqualLine = m_line.contains(rxEqual);
        bool currIsDashLine  = m_line.contains(rxDash);

        // Keep a record of the history
        if (m_line.isEmpty()) {
            addToHistory(EmptyLine, m_line);
        } else if (currIsDashLine) {
            addToHistory(DashLine, m_line);
        } else if (currIsEqualLine) {
            addToHistory(EqualLine, m_line);
        } else {
            addToHistory(NormalLine, m_line);
        }

        // Waste some memory to increase readability
        const int line0Type = m_lineTypeHistory.at(0); // Oldest line
        const int line1Type = m_lineTypeHistory.at(1);
        const int line2Type = m_lineTypeHistory.at(2); // Current line

        // Check for Paragraph begin
        if (m_paraLineNumber < 0) {
            if (line1Type == NormalLine) {
                paraLine = m_lineHistory.at(1);
                m_paraLineNumber = lineNumber() - 1;
            } else if (line2Type == NormalLine) {
                paraLine = m_lineHistory.at(2);
                m_paraLineNumber = lineNumber();
            } else {
                continue;
            }
        }

        // Special checks for ISO date header. They to add in a sane way and give them
        // an own LineType proved to be surprisingly complicated, so it's done quirky.
        // Thanks to https://stackoverflow.com/a/46362201
        bool line0IsDate = m_lineHistory.at(0).contains(rxIsoDate);
        bool line1IsDate = m_lineHistory.at(1).contains(rxIsoDate);

        if (line0IsDate && line1Type == EqualLine && line2Type == NormalLine && lastNode()) {
            QString mask(QStringLiteral("%1 %2"));
            lastNode()->setText(0, mask.arg(m_lineHistory.at(0)).arg(m_lineHistory.at(2)));
            initHistory();

        } else if (line0Type != NormalLine && line1IsDate && line2Type == NormalLine) {
            QString mask(QStringLiteral("%1 %2"));
            addNode(SectNode, mask.arg(m_lineHistory.at(1)).arg(m_lineHistory.at(2)), lineNumber() - 1);
            initHistory();
            //  [End] Special checks for ISO date header

            // Check for Paragraph continuation
        } else if (line0Type == NormalLine && line1Type == NormalLine  && line2Type == NormalLine) {
            continue;

            // Check for Paragraph - Single line
        } else if (line0Type != NormalLine && line1Type == NormalLine  && line2Type == EmptyLine) {
            addNode(ParaNode, paraLine, m_paraLineNumber);

            // Check for Paragraph - Two or more lines
        } else if (line0Type == NormalLine && line1Type == NormalLine  && line2Type != NormalLine) {
            addNode(ParaNode, paraLine, m_paraLineNumber);

            // Check for Header
        } else if (line0Type != NormalLine && line1Type == NormalLine && line2Type == DashLine) {
            addNode(HeadNode, m_lineHistory.at(1), lineNumber() - 1);

            // Check for Section
        } else if (line0Type != NormalLine && line1Type == NormalLine && line2Type == EqualLine) {
            addNode(SectNode, m_lineHistory.at(1), lineNumber() - 1);
        }
    }

    fnishEndlines();
}


IniFileParser::IniFileParser(QObject *view, KTextEditor::Document *doc)
    : DocumentParser(view, doc)
{
    using namespace IconCollection;
    registerViewOption(Section1Node, Head1Icon, QStringLiteral("Header1"), i18n("Show Header 1"));
    registerViewOption(Section2Node, Head2Icon, QStringLiteral("Header2"), i18n("Show Header 2"));
    registerViewOption(Section3Node, Head3Icon, QStringLiteral("Header3"), i18n("Show Header 3"));
    registerViewOption(Section4Node, Head4Icon, QStringLiteral("Header4"), i18n("Show Header 4"));
    registerViewOption(Section5Node, Head5Icon, QStringLiteral("Header5"), i18n("Show Header 5"));
    registerViewOption(Section6Node, Head6Icon, QStringLiteral("Header6"), i18n("Show Header 6"));
}


IniFileParser::~IniFileParser()
{
}


void IniFileParser::parseDocument()
{
    static const QRegularExpression rxBrackets(QStringLiteral(R"(^\[(.+)\]$)"));
    static const QRegularExpression rxSubSect(QStringLiteral(R"([:])"));
//  static const QRegularExpression rxSubSect(QStringLiteral(R"([:.-])")); <= FIXME too simple, see below

    // https://en.wikipedia.org/wiki/INI_file
    //      ; comment text
    //      # comment text
    //      [section]
    //      [section.subsection]                                 <= So far no problem, but disabled
    //      [.subsection]                                        <= TODO
    //
    // ~/.kde4/share/config/kdeglobals
    //      [Colors:Header][Inactive]
    //
    // ~/.local/share/kate/sessions/Foo.katesession
    //      [Plugin:kateindexviewplugin:MainWindow:0]
    //      [MainWindow0]
    //      [MainWindow0 Settings]
    //      [MainWindow0-Splitter 0]
    //      [MainWindow0-ViewSpace 0]
    //      [MainWindow0-ViewSpace 0 file:///some/url/foo.bar]   <= Make nice trouble

    initHistory(1);

    QRegularExpressionMatch rxMatch;

    QTreeWidgetItem *docRoot = rootNode(RootNode);

    while (nextLine()) {
        // Let's start the investigation
        if (!m_line.contains(rxBrackets, &rxMatch)) {
            continue;
        }
//         qDebug() << lineNumber() << rxMatch.captured(1);
//         qDebug() << lineNumber() << m_line;

        m_sections.clear();

        // Split the found bracket into it's section parts, if some
        QStringList bracketSections = rxMatch.captured(1).split(QStringLiteral("]["));
        for (int i = 0; i < bracketSections.size(); ++i) {
            QStringList subSections = bracketSections.at(i).split(rxSubSect);
            for (int j = 0; j < subSections.size(); ++j) {
                m_sections << subSections.at(j);
            }
        }

//         qDebug() << lineNumber() << m_sections;

        // We can't assume that the now collected "path" is already in it's logical position
        // so we must look at our tree if we find some existing top section
        QTreeWidgetItem *node = docRoot;
        int i = 0;
        bool sectionExist = false;
        while (i < m_sections.size()) {
            for (int j = 0; j < node->childCount(); ++j) {
//                 qDebug() << "COMPARE" << i << j << node->child(j)->text(0) << m_sections.at(i);
                if (node->child(j)->text(0) == m_sections.at(i)) {
                    node = node->child(j);
                    sectionExist = true;
                    break;
                }
            }
            if (!sectionExist) {
                lastNode()->setData(0, NodeData::EndLine, lineNumber() - 1);
                addNodesToParent(node, i);
                break;
            }
            sectionExist = false;
            ++i;
        }
    }
}


void IniFileParser::addNodesToParent(QTreeWidgetItem *parentNode, int pos)
{
    for (int i = pos; i < m_sections.size(); ++i) {
        parentNode = addNodeToParent(i+1, parentNode, m_sections.at(i));
    }
}


QTreeWidgetItem *IniFileParser::addNodeToParent(int nodeType, QTreeWidgetItem *parentNode, const QString &text)
{
//     qDebug() << "ADD " << text << "type" << nodeType << "TO" << parentNode->text(0);

    QTreeWidgetItem *node = new QTreeWidgetItem(parentNode, nodeType);
    setNodeProperties(node, nodeType, text, lineNumber());

    return lastNode();
}


DiffFileParser::DiffFileParser(QObject *view, KTextEditor::Document *doc)
    : DocumentParser(view, doc)
{
    using namespace IconCollection;
    setNodeTypeIcon(Section1Node, Head1Icon);
    setNodeTypeIcon(Section2Node, Head2Icon);
    setNodeTypeIcon(Section3Node, Head3Icon);
    setNodeTypeIcon(Section4Node, Head4Icon);
    setNodeTypeIcon(Section5Node, Head5Icon);
    setNodeTypeIcon(Section6Node, Head6Icon);
    setNodeTypeIcon(ChunkNode, ParaIcon);

    m_noNumberAsChunk = addViewOption(QStringLiteral("NoNumberAsChunk"), i18n("Avoid line number as chunk"));

    // Ignore empty lines in general
    m_rxIgnoreLine = QRegularExpression(QStringLiteral(R"(^\s*$)"));
}


DiffFileParser::~DiffFileParser()
{
}


void DiffFileParser::parseDocument()
{
    // Match: diff --xyz a/index-view/plaintext_parser.cpp b/index-view/plaintext_parser.cpp
    //    or: diff ----- a/index-view/plaintext_parser.cpp b/index-view/plaintext_parser.cpp
    static const QRegularExpression rxUrl1(QStringLiteral(R"(^diff\ [-]+\w*\ a(.+)\ b(.+))"));
    // Match: diff --xyz index-view/parser.cpp
    // Match: diff ----- index-view/parser.cpp
    static const QRegularExpression rxUrl2(QStringLiteral(R"(^diff\ [-]+\w*\ (.+))"));
    static const QRegularExpression rxChunks(QStringLiteral(R"(^@@(.*)@@(.*))"));

    QRegularExpressionMatch rxMatch;
    QStringList m_inputPath;
    QTreeWidgetItem *fileNode = nullptr;

    while (nextLine()) {
        // Let's start the investigation
        if (m_line.contains(rxUrl1, &rxMatch)) {
            m_inputPath = rxMatch.captured(1).split(QLatin1Char('/'), Qt::SkipEmptyParts);
            if (!fileNode || fileNode->text(0) != m_inputPath.at(m_inputPath.size()-1)) {
                fileNode =  addLimbToNode(Section1Node, Section6Node, m_inputPath);
            }

        } else if (m_line.contains(rxUrl2, &rxMatch)) {
            m_inputPath = rxMatch.captured(1).split(QLatin1Char('/'), Qt::SkipEmptyParts);
            if (!fileNode || fileNode->text(0) != m_inputPath.at(m_inputPath.size()-1)) {
                fileNode =  addLimbToNode(Section1Node, Section6Node, m_inputPath);
            }

        } else if (m_line.contains(rxChunks, &rxMatch)) {
            if (m_noNumberAsChunk->isChecked()) {
                if (rxMatch.captured(2).isEmpty()) {
                    int lineNo = lineNumber();
                    nextLine();
                    addNodeToParent(ChunkNode, fileNode, m_line.simplified());
                    lastNode()->setData(0, NodeData::Line, lineNo);
                } else {
                    addNodeToParent(ChunkNode, fileNode, rxMatch.captured(2).simplified());
                }
            } else {
                addNodeToParent(ChunkNode, fileNode, rxMatch.captured(1));
            }
        }
    }
}


// kate: space-indent on; indent-width 4; replace-tabs on;
