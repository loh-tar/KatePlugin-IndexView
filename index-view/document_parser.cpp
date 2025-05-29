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


DocumentParser::DocumentParser(QObject *view, KTextEditor::Document *doc)
    : Parser(view, doc)
{
    useNestingOptions(true/* with adjustment of nesting level*/);

    setNodeTypeIcon(RootNode, IconCollection::DocumentRootIcon);
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

    if (lastNode()) {
        lastNode()->setData(0, NodeData::EndLine, lineNumber() - 1);
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
            m_line = nextLineOrBust();
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

        // Ignore some lines to avoid to start a new paragraph
        // or to break a continuation paragraph. Useful with some config files
        if (m_line.size() == 1) {
            continue;
        } else if (m_line.contains(m_rxIgnoreLine)) {
            continue;
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
        QAction *viewOption = p_nodeTypes.value(i).option;
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
    resetNesting();

    p_indexTree->setRootIsDecorated(1);

    // Add the root node here keeps addNode() less complex
    // Using line number 0 will cause a jump to the top of the document when
    // clicked which make absolutely sense.
    QTreeWidgetItem *node = rootNode(RootNode);
    setNodeProperties(node, RootNode, i18n("Document"), 0);
}


void DocumentParser::fnishEndlines()
{
    QTreeWidgetItem *node = lastNode();
    while (node) {
        //qDebug() << "fnishEndlines UPDATE EndLine" << node->text(0) << "from" << node->data(0, NodeData::EndLine).toInt() << "to" << lineNumber() -1;
        node->setData(0, NodeData::EndLine, lineNumber() - 1);
        node = node->parent();
    }
}


void DocumentParser::addNode(const int nodeType, const QString &text, const int lineNumber)
{
    QTreeWidgetItem *node = nullptr;

    // Indicate, there is no paragraph waiting for completion
    m_paraLineNumber = -1;

    if (lastNode()) {
        ++p_nestingLevel;
        QTreeWidgetItem *parentNode = lastNode();
        //qDebug() << "UPDATE   LAST" << parentNode->text(0) << "from" << parentNode->data(0, NodeData::EndLine).toInt() << "to" << fromLine -1;
        parentNode->setData(0, NodeData::EndLine, lineNumber - 1);
        while (parentNode->type() >= nodeType) {
            parentNode = parentNode->parent();
            //qDebug() << "UPDATE PARENT" << parentNode->text(0) << "from" << parentNode->data(0, NodeData::EndLine).toInt() << "to" << lineNumber -1;
            parentNode->setData(0, NodeData::EndLine, lineNumber - 1);
            --p_nestingLevel;
        }
        node = new QTreeWidgetItem(parentNode, nodeType);
    } else {
        qDebug() << "DocumentParser::addNode - surprising, should never happens!";
        node = new QTreeWidgetItem(p_indexTree, nodeType);
    }

    setNodeProperties(node, nodeType, text, lineNumber);
    //qDebug() << "new node" <<  lineNumber << text;
}


QTreeWidgetItem *DocumentParser::addLimbToNode(int firstNodeType, int lastNodeType, const QStringList &nodeList, QTreeWidgetItem *branchNode/* = nullptr*/)
{
    if (!branchNode) {
        branchNode = rootNode(RootNode);
    }

//     auto addNodeToParent = [this](int nodeType, QTreeWidgetItem *parentNode, const QString &text) {
//         QTreeWidgetItem *node = new QTreeWidgetItem(parentNode, nodeType);
//         setNodeProperties(node, nodeType, text, lineNumber());
//         return lastNode();
//     };

    QTreeWidgetItem *node = branchNode;
    int i = 0;
    bool sectionExist = false;
    while (i < nodeList.size()) {
        for (int j = 0; j < node->childCount(); ++j) {
            //                 qDebug() << "COMPARE" << i << j << node->child(j)->text(0) << nodeList.at(i);
            if (node->child(j)->text(0) == nodeList.at(i)) {
                node = node->child(j);
                sectionExist = true;
                break;
            }
        }
        if (!sectionExist) {
            lastNode()->setData(0, NodeData::EndLine, lineNumber() - 1);
            int nodeType = firstNodeType;
            for (int k = i; k < nodeList.size(); ++k) {
                node = addNodeToParent(k+1, node, nodeList.at(k));
                nodeType = qMin(nodeType + 1, lastNodeType);
            }
            break;
        }
        sectionExist = false;
        ++i;
    }

    return lastNode();
}

QTreeWidgetItem *DocumentParser::addNodeToParent(int nodeType, QTreeWidgetItem *parentNode, const QString &text)
{
    if (!parentNode) {
        qDebug() << "DocumentParser::addNodeToParent FATAL parent is nullptr" << text << "type" << nodeType;
//     } else {
//         qDebug() << "ADD " << text << "type" << nodeType << "TO" << parentNode->text(0);
    }

    QTreeWidgetItem *node = new QTreeWidgetItem(parentNode, nodeType);
    setNodeProperties(node, nodeType, text, lineNumber());

    return lastNode();
}


void DocumentParser::initHistory(int size /*= -1*/)
{
    if (size > -1) {
        p_historySize = size;
    }

    m_lineTypeHistory.clear();
    m_lineHistory.clear();
    for (int i = 0; i < p_historySize; i++) {
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
