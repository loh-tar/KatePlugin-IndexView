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


MarkdownParser::MarkdownParser(QObject *view, const QString &docType, KTextEditor::Document *doc)
    : DocumentParser(view, docType, doc)
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
    static const QRegularExpression rx1(QStringLiteral("^[=]{3,}$"));
    static const QRegularExpression rx2(QStringLiteral("^[-]{3,}$"));
    static const QRegularExpression rx3(QStringLiteral("^#{1,6}\\s.*$"));

    QString paraLine;   // First line of a paragraph
    initHistory(3);

    while (nextLine()) {
        // Let's start the investigation
        bool currIsEqualLine = rawLine().contains(rx1);
        bool currIsDashLine  = rawLine().contains(rx2);
        bool currIsHeader    = rawLine().contains(rx3);

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
        }
    }

    fnishEndlines();
}


AsciiDocParser::AsciiDocParser(QObject *view, const QString &docType, KTextEditor::Document *doc)
    : DocumentParser(view, docType, doc)
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
    static const QRegularExpression rx1(QStringLiteral("^[=]{3,}$"));
    static const QRegularExpression rx2(QStringLiteral("^[-]{3,}$"));
    static const QRegularExpression rx3(QStringLiteral("^={1,6}\\s.*$"));

    QString paraLine;   // First line of a paragraph
    initHistory(3);

    while (nextLine()) {
        // Let's start the investigation
        bool currIsEqualLine = rawLine().contains(rx1);// atm not used (and related)
        bool currIsDashLine  = rawLine().contains(rx2);
        bool currIsHeader    = rawLine().contains(rx3);

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
