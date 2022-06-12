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

#include "plaintext_parser.h"


PlainTextParser::PlainTextParser(IndexView *view)
    : DocumentParser(view)
{
    using namespace IconCollection;
    registerViewOption(SectNode, Head1Icon, QStringLiteral("Section"), i18n("Show Sections"));
    registerViewOption(HeadNode, Head2Icon, QStringLiteral("Header"), i18n("Show Headers"));
    registerViewOption(ParaNode, ParaIcon, QStringLiteral("Paragraph"), i18n("Show Paragraphs"));
}


PlainTextParser::~PlainTextParser()
{
}


void PlainTextParser::parseDocument()
{
    QString paraLine;   // First line of a paragraph

    initHistory(3);

    while (nextLine()) {
        // Let's start the investigation
        bool currIsEqualLine = m_line.contains(QRegularExpression(QStringLiteral("^[=#*]{3,}$")));
        bool currIsDashLine  = m_line.contains(QRegularExpression(QStringLiteral("^[-~^]{3,}$")));

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
        const int lineType0 = m_lineTypeHistory.at(0); // Oldest line
        const int lineType1 = m_lineTypeHistory.at(1);
        const int lineType2 = m_lineTypeHistory.at(2); // Current line

        // Check for Paragraph begin
        if (m_paraLineNumber < 0) {
            if (lineType1 == NormalLine) {
                paraLine = m_lineHistory.at(1);
                m_paraLineNumber = lineNumber() - 1;
            } else if (lineType2 == NormalLine) {
                paraLine = m_lineHistory.at(2);
                m_paraLineNumber = lineNumber();
            } else {
                continue;
            }
        }

        // Special checks for ISO date header. They to add in a sane way and give them
        // an own LineType proved to be surprisingly complicated, so it's done quirky.
        // Thanks to https://stackoverflow.com/a/46362201
        QRegularExpression isoDate(QStringLiteral("^\\d{4}-([0]\\d|1[0-2])-([0-2]\\d|3[01])$"));
        bool line0IsDate = m_lineHistory.at(0).contains(isoDate);
        bool line1IsDate = m_lineHistory.at(1).contains(isoDate);

        if (line0IsDate && lineType1 == EqualLine && lineType2 == NormalLine && lastNode()) {
            QString mask(QStringLiteral("%1 %2"));
            lastNode()->setText(0, mask.arg(m_lineHistory.at(0)).arg(m_lineHistory.at(2)));
            initHistory();

        } else if (lineType0 != NormalLine && line1IsDate && lineType2 == NormalLine) {
            QString mask(QStringLiteral("%1 %2"));
            addNode(SectNode, mask.arg(m_lineHistory.at(1)).arg(m_lineHistory.at(2)), lineNumber() - 1);
            initHistory();
            //  [End] Special checks for ISO date header

            // Check for Paragraph continuation
        } else if (lineType0 == NormalLine && lineType1 == NormalLine  && lineType2 == NormalLine) {
            continue;

            // Check for Paragraph - Single line
        } else if (lineType0 != NormalLine && lineType1 == NormalLine  && lineType2 == EmptyLine) {
            addNode(ParaNode, paraLine, m_paraLineNumber);

            // Check for Paragraph - Two or more lines
        } else if (lineType0 == NormalLine && lineType1 == NormalLine  && lineType2 != NormalLine) {
            addNode(ParaNode, paraLine, m_paraLineNumber);

            // Check for Header
        } else if (lineType0 != NormalLine && lineType1 == NormalLine && lineType2 == DashLine) {
            addNode(HeadNode, m_lineHistory.at(1), lineNumber() - 1);

            // Check for Section
        } else if (lineType0 != NormalLine && lineType1 == NormalLine && lineType2 == EqualLine) {
            addNode(SectNode, m_lineHistory.at(1), lineNumber() - 1);
        }
    }
}

// kate: space-indent on; indent-width 4; replace-tabs on;
