/*   This file is part of KatePlugin-IndexView
 *
 *   DocumentParser Class
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


#include <QDebug>

#include <KLocalizedString>

#include "icon_collection.h"
#include "index_view.h"

#include "document_parser.h"


DocumentParser::DocumentParser(IndexView *view)
    : Parser(view)
{
}


DocumentParser::~DocumentParser()
{
}


void DocumentParser::parse()
{
    const QString checksum = disableDependentOptions();

    Parser::parse();

    // Hiding not useful options is surprising tricky, we need to parse twice
    // in some cases to ensure everything works as desired
    if (checksum != disableDependentOptions()) {
        Parser::parse();
    }
}


bool DocumentParser::nextLine()
{
    while (true) {
        // Run the loop one line more as usually needed...
        if (p_lineNumber >= (p_document->lines() + 1)) {
            return false;
        }

        // ...to ensure a last paragraph is added properly...
        if (p_lineNumber < p_document->lines()) {
            m_line = p_document->line(p_lineNumber++);
            m_line = m_line.trimmed();
            m_line = m_line.simplified();
            m_line.truncate(80); // Limit the size to some acceptable length
        } else {
            ++p_lineNumber;
            if (m_paraLineNumber == -1) {
                // Good, we are done!
                break;
            }
            // ...when there was no \n at the last line
            m_line.clear();
        }

        // Treat some lines as empty lines to avaoid to start a new paragraph
        // or to break a continuation paragraph. Useful with some config files
        if (m_line.size() == 1) {
            m_line.clear();
        } else if (m_line.contains(m_rxIgnoreLine)) {
            m_line.clear();
        }

        if (m_line.isEmpty() && m_lineTypeHistory.last() == EmptyLine) {
            // Ignore consecutive empty lines
            continue;
        }

        break;
    }

    return true;
}


QString DocumentParser::disableDependentOptions()
{
    QString checksum;
    bool    somePrevIsAvailable = true;

    for(int i = 0; true; i++) {
        QAction *viewOption = p_viewOptions.value(i);
        if (!viewOption) {
            break;
        }
        if (!viewOption->isVisible()) {
            continue;
        }

        if (somePrevIsAvailable) {
            checksum.append(viewOption->objectName());
        }
        viewOption->setEnabled(somePrevIsAvailable);
        somePrevIsAvailable = viewOption->isChecked() && viewOption->isEnabled();
    }

    return checksum;
}


void DocumentParser::prepareForParse()
{
    if (!p_viewTree->isChecked()) {
        p_indexTree->setRootIsDecorated(0);
        return;
    }

    p_indexTree->setRootIsDecorated(1);

    // What here is done is like registerViewOption(int, int, QString, QString)
    // but then would there a pointless option to disable the root node in the menu
    // and that option will be saved, regardless if we hide the option.
    // So we go this quirky way.
    QAction *viewOption = new QAction(this);
    viewOption->setCheckable(true);
    viewOption->setChecked(true);

    p_icons.insert(RootNode, IconCollection::getIcon(IconCollection::BlueIcon));
    p_viewOptions.insert(RootNode, viewOption);

    // Add the root node here keeps addNode() less complex
    // Using line number 0 will cause a jump to the top of the document when
    // clicked which make absolutely sense.
    QTreeWidgetItem *node = new QTreeWidgetItem(p_indexTree, RootNode);
    setNodeProperties(node, RootNode, i18n("Document"), 0);
}


void DocumentParser::addNode(const int nodeType, const QString &text, const int lineNumber)
{
    QTreeWidgetItem *node = nullptr;

    // Indicate, there is no paragraph waiting for completion
    m_paraLineNumber = -1;

    if (!nodeTypeIsWanted(nodeType)) {
        return;
    }

    if (p_viewTree->isChecked()) {
        if (lastNode()) {
            QTreeWidgetItem *parentNode = lastNode();
            while (parentNode->type() >= nodeType) {
                parentNode = parentNode->parent();
            }
            node = new QTreeWidgetItem(parentNode, nodeType);
        } else {
            qDebug() << "DocumentParser::addNode - surprising, should never happens!";
            node = new QTreeWidgetItem(p_indexTree, nodeType);
        }

    } else {
        node = new QTreeWidgetItem(p_indexTree, nodeType);
    }

    setNodeProperties(node, nodeType, text, lineNumber);
}


void DocumentParser::initHistory(int size /*= -1*/)
{
    if (size > -1) {
        p_historySize = size;
    }

    m_lineTypeHistory.clear();
    m_lineHistory.clear();
    for (int i = 0; i < size; i++) {
        m_lineTypeHistory.enqueue(EmptyLine);
        m_lineHistory.enqueue(QStringLiteral(""));
    }

    m_paraLineNumber = -1;
}


void DocumentParser::addToHistory(int lineType, const QString &line)
{
    m_lineTypeHistory.enqueue(lineType);
    m_lineHistory.enqueue(line);

    // Limit the history
    if (m_lineHistory.size() > p_historySize) {
        m_lineHistory.dequeue();
        m_lineTypeHistory.dequeue();
    }
}

// kate: space-indent on; indent-width 4; replace-tabs on;
