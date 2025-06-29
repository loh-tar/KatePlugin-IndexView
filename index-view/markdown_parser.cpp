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


#include <KLocalizedString>

#include "icon_collection.h"

#include "markdown_parser.h"


MarkdownParser::MarkdownParser(QObject *view, KTextEditor::Document *doc)
    : DocumentParser(view, doc)
{
    using namespace IconCollection;
    registerViewOption(ParaNode, ParaIcon, QStringLiteral("Paragraph"), i18n("Show Paragraphs"));
    registerViewOption(LinkNode, LinkIcon, QStringLiteral("Links and Footnotes"), i18n("Show Links and Footnotes"));

    p_detachLinks = registerViewOptionModifier(LinkNode, QStringLiteral("DetachLinks"), i18n("Detach Links and Footnotes"));

    setNodeTypeIcon(Head1Node, Head1Icon);
    setNodeTypeIcon(Head2Node, Head2Icon);
    setNodeTypeIcon(Head3Node, Head3Icon);
    setNodeTypeIcon(Head4Node, Head4Icon);
    setNodeTypeIcon(Head5Node, Head5Icon);
    setNodeTypeIcon(Head6Node, Head6Icon);
}


MarkdownParser::~MarkdownParser()
{
}


// MarkdownParser is massive based on PlainTextParser
// The main differences are:
//   Enhanced LineType enum
//   Enhanced NodeType enum
//   Add check for HeaderLine
//   Removed quirk for ISO headers
//   Removed #~^ as header underlining

void MarkdownParser::parseDocument()
{
    static const QRegularExpression rxEqualLine(QStringLiteral(R"(^[=]{3,}$)"));
    static const QRegularExpression rxDashLine(QStringLiteral(R"(^[-]{3,}$)"));
    static const QRegularExpression rxHeader(QStringLiteral(R"(^#{1,6}\s.*$)"));
    static const QRegularExpression rxLinkLine(QStringLiteral(R"(^( {0,3})\[(.*)\]:(\t| )+(.*)$)"));
    static const QRegularExpression rxCodeLine(QStringLiteral(R"(^(\t| {4,})+\S.*$)"));
    static const QRegularExpression rxIndentLine(QStringLiteral(R"(^( {2,})+\S.*$)"));

    QString paraLine;   // First line of a paragraph
    initHistory(3);

    if (p_detachLinks->isChecked()) {
        m_detachedNodeTypes << LinkNode;
    } else {
        m_detachedNodeTypes.remove(LinkNode);
    }

    while (nextLine()) {
        // Let's start the investigation
        bool currIsEqualLine = rawLine().contains(rxEqualLine);
        bool currIsDashLine  = rawLine().contains(rxDashLine);
        bool currIsHeader    = rawLine().contains(rxHeader);
        bool currIsLink      = rawLine().contains(rxLinkLine);
        bool currIsCode      = rawLine().contains(rxCodeLine);
        bool currIsIndented  = rawLine().contains(rxIndentLine);

        // Keep a record of the history
        if (m_line.isEmpty()) {
            addToHistory(EmptyLine, m_line);
        } else if (currIsDashLine) {
            addToHistory(DashLine, m_line);
        } else if (currIsEqualLine) {
            addToHistory(EqualLine, m_line);
        } else if (currIsHeader) {
            addToHistory(HeaderLine, m_line);
        } else if (currIsLink) {
            addToHistory(LinkLine, m_line);
        } else if (currIsCode) {
            // More is not needed to ignore code lines "by Gruber", just this notice
            addToHistory(CodeLine, m_line);

            // Due to the similarity with rxCodeLine we must check after currIsCode
            // HINT: Looks pointless, a modified version of rxCodeLine should do it too
            // but who knows what we can further improve...
        } else if (currIsIndented) {
            // More is not needed to ignore indent lines, just this notice
            addToHistory(IndentLine, m_line);
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
            }
        }

        // Check for Paragraph continuation
        if (line0Type == NormalLine && line1Type == NormalLine  && line2Type == NormalLine) {
            continue;

        } else if (line2Type  == LinkLine) {
            if (p_detachLinks->isChecked()) {
                addDetachedNode(LinkNode, m_line, lineNumber());
            } else {
                QTreeWidgetItem *parentNode = lastNode();
                while (parentNode) {
                    if (parentNode->type() <= Head6Node) {
                        break;
                    }
                    parentNode = parentNode->parent();
                }
                addNodeToParent(LinkNode, parentNode, m_line);
            }

            // Check for Paragraph - Single line
        } else if (line0Type != NormalLine && line1Type == NormalLine  && line2Type  == EmptyLine) {
            addNode(ParaNode, paraLine, m_paraLineNumber);

            // Check for Paragraph - Two or more lines
        } else if (line0Type == NormalLine && line1Type == NormalLine  && line2Type != NormalLine) {
            addNode(ParaNode, paraLine, m_paraLineNumber);

            // Check for Setext-style header H2
        } else if (line0Type != NormalLine && line1Type == NormalLine && line2Type == DashLine) {
            addNode(Head2Node, m_lineHistory.at(1), lineNumber() - 1);

            // Check for Setext-style header H1
        } else if (line0Type != NormalLine && line1Type == NormalLine && line2Type == EqualLine) {
            addNode(Head1Node, m_lineHistory.at(1), lineNumber() - 1);

            // Check for sharp # header
        } else if (line2Type == HeaderLine) {
            QString text = m_lineHistory.at(2).section(QLatin1Char(' '), 1);
            int    level = m_lineHistory.at(2).section(QLatin1Char(' '), 0, 0).size();
            NodeType headerType;
            switch (level) {
            case 1: headerType = Head1Node; break;
            case 2: headerType = Head2Node; break;
            case 3: headerType = Head3Node; break;
            case 4: headerType = Head4Node; break;
            case 5: headerType = Head5Node; break;
            // There can only be case 6, but anyway we use default
            default: headerType = Head6Node; break;
            }
            addNode(headerType, text, lineNumber());

            // Check for github code blocks to be skipped
        } else if (rawLine().startsWith(QStringLiteral("```"))) {
            while (nextLine()) {
                if (!rawLine().startsWith(QStringLiteral("```"))) {
                    continue;
                }
                initHistory();
                break;
            }

            // Check for pre blocks to be skipped
        } else if (rawLine().startsWith(QStringLiteral("<pre>"))) {
            while (nextLine()) {
                if (!rawLine().startsWith(QStringLiteral("</pre>"))) {
                    continue;
                }
                initHistory();
                break;
            }
        }
    }

    fnishEndlines();
}


AsciiDocParser::AsciiDocParser(QObject *view, KTextEditor::Document *doc)
    : DocumentParser(view, doc)
{
    using namespace IconCollection;
    registerViewOption(ParaNode, ParaIcon, QStringLiteral("Paragraph"), i18n("Show Paragraphs"));

    setNodeTypeIcon(Head1Node, Head1Icon);
    setNodeTypeIcon(Head2Node, Head2Icon);
    setNodeTypeIcon(Head3Node, Head3Icon);
    setNodeTypeIcon(Head4Node, Head4Icon);
    setNodeTypeIcon(Head5Node, Head5Icon);
    setNodeTypeIcon(Head6Node, Head6Icon);
}


AsciiDocParser::~AsciiDocParser()
{
}


// AsciiDocParser is massive based on MarkdownParser
// The main differences are:
//   Removed Setext-style header
//
// TODO How about an "Strict" option where we refuse to show bad nested headers?
// https://docs.asciidoctor.org/asciidoc/latest/sections/titles-and-levels/
// HINT: There is noted that "Asciidoctor recognizes the hash symbol (#) from Markdown"
//       But I understood this more as a "we try to be handy feature" and not be part of
//       the AsciiDoc spec, so I think we should not support these too

void AsciiDocParser::parseDocument()
{
    static const QRegularExpression rxEqualLine(QStringLiteral(R"(^[=]{3,}$)"));
    static const QRegularExpression rxDashLine(QStringLiteral(R"(^[-]{3,}$)"));
    static const QRegularExpression rxHeader(QStringLiteral(R"(^={1,6}\s.*$)"));

    QString paraLine;   // First line of a paragraph
    initHistory(3);

    while (nextLine()) {
        // Let's start the investigation
        bool currIsEqualLine = rawLine().contains(rxEqualLine);// atm not used (and related)
        bool currIsDashLine  = rawLine().contains(rxDashLine);
        bool currIsHeader    = rawLine().contains(rxHeader);

        // Keep a record of the history
        if (m_line.isEmpty()) {
            addToHistory(EmptyLine, m_line);
        } else if (currIsDashLine) {
            addToHistory(DashLine, m_line);
        } else if (currIsEqualLine) {
            addToHistory(EqualLine, m_line);
        } else if (currIsHeader) {
            addToHistory(HeaderLine, m_line);
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
            } else if (line2Type != HeaderLine) {
                continue;
            }
        }

        // Check for Paragraph continuation
        if (line0Type == NormalLine && line1Type == NormalLine  && line2Type == NormalLine) {
            continue;

            // Check for Paragraph - Single line
        } else if (line0Type != NormalLine && line1Type == NormalLine  && line2Type  == EmptyLine) {
            addNode(ParaNode, paraLine, m_paraLineNumber);

            // Check for Paragraph - Two or more lines
        } else if (line0Type == NormalLine && line1Type == NormalLine  && line2Type != NormalLine) {
            addNode(ParaNode, paraLine, m_paraLineNumber);

            // Check for sharp # header
        } else if (line2Type == HeaderLine) {
            QString text = m_lineHistory.at(2).section(QLatin1Char(' '), 1);
            int    level = m_lineHistory.at(2).section(QLatin1Char(' '), 0, 0).size();
            NodeType headerType;
            switch (level) {
            case 1: headerType = Head1Node; break;
            case 2: headerType = Head2Node; break;
            case 3: headerType = Head3Node; break;
            case 4: headerType = Head4Node; break;
            case 5: headerType = Head5Node; break;
            // There can only be case 6, but anyway we use default
            default: headerType = Head6Node; break;
            }
            addNode(headerType, text, lineNumber());
        }
    }

    fnishEndlines();
}

// kate: space-indent on; indent-width 4; replace-tabs on;
