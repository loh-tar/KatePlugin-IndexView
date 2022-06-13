/*   This file is part of KatePlugin-IndexView
 *
 *   Parser Class
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

#include "bash_parser.h"
#include "cpp_parser.h"
#include "ecma_parser.h"
#include "fortran_parser.h"
#include "markdown_parser.h"
#include "perl_parser.h"
#include "php_parser.h"
#include "plaintext_parser.h"
#include "python_parser.h"
#include "ruby_parser.h"
#include "tcl_parser.h"
#include "xslt_parser.h"

#include "parser.h"


DummyParser::DummyParser(const QString &type, IndexView *view)
    : Parser(view)
    , m_docType(type)
{
    using namespace IconCollection;
    registerViewOption(InfoNode, RedBlueIcon, QStringLiteral("Info"), i18n("Show Info"));

    // Hide all possible general options to deactivate the menu
    p_viewTree->setVisible(false);
    p_addIcons->setVisible(false);
    p_viewExpanded->setVisible(false);
}


DummyParser::~DummyParser()
{
}


void DummyParser::addNode(const int nodeType, const QString &text, const int lineNumber)
{
    QTreeWidgetItem *node = new QTreeWidgetItem(p_indexTree, nodeType);

    // Don't fill p_usefulOptions, so option will hidden
    // QAction *viewOption = p_viewOptions.value(nodeType, nullptr);
    // p_usefulOptions.insert(viewOption);

    if (node) {
        node->setText(0, text);
        // Don't set icon, looks odd
        // node->setIcon(0, p_icons.value(nodeType));
        node->setData(0, NodeData::Line, lineNumber);
    }
}


void DummyParser::parseDocument()
{
    p_indexTree->setRootIsDecorated(0);
    addNode(InfoNode, i18n("Sorry, not supported yet!"), -1);
    addNode(InfoNode, i18n("File type: %1", m_docType), -1);
}


Parser::Parser(IndexView *view)
    : QObject(view)
    , p_document(view->m_mainWindow->activeView()->document())
    , p_indexTree(view->m_indexTree)
    , p_view(view)
{
    view->m_viewSort = addViewOption(QStringLiteral("SortIndex"), i18n("Show Sorted"));

    p_viewTree     = addViewOption(QStringLiteral("TreeView"), i18n("Tree View"));
    p_addIcons     = addViewOption(QStringLiteral("AddIcons"), i18n("Adorn View"));
    p_viewExpanded = addViewOption(QStringLiteral("ExpandView"), i18n("Expand View"));

    view->m_viewTree = p_viewTree;
    view->m_viewExpanded = p_viewExpanded;

    addViewOptionSeparator();

    p_modifierOptions.insert(p_viewExpanded, p_viewTree);
}


Parser::~Parser()
{
}


Parser *Parser::create(const QString &type, IndexView *view)
{
// qDebug() << "Parser::create" << type;
    // FIXME Do you know some more elegant way to achieve the desired result?
    // The bad things here are, that you must add here each new parser and recompile
    // at least this plugin

    // Ordered by parser class name, except...
    if (type == QStringLiteral("Bash"))
        return new BashParser(view);
    else if (type == QStringLiteral("C++") || type == QStringLiteral("C") || type == QStringLiteral("ANSI C89") || type == QStringLiteral("Java"))
        return new CppParser(view);
    else if (type == QStringLiteral("ActionScript 2.0") || type == QStringLiteral("JavaScript") || type == QStringLiteral("QML"))
        return new EcmaParser(view);
    else if (type == QStringLiteral("Fortran"))
        return new FortranParser(view);
    else if (type == QStringLiteral("Markdown"))
        return new MarkdownParser(view);
    else if (type == QStringLiteral("Perl"))
        return new PerlParser(view);
    else if (type == QStringLiteral("PHP (HTML)"))
        return new PhpParser(view);
    else if (type == QStringLiteral("Normal") || type == QStringLiteral(".desktop"))
        return new PlainTextParser(view);
    else if (type == QStringLiteral("Python"))
        return new PythonParser(view);
    else if (type == QStringLiteral("Ruby"))
        return new RubyParser(view);
    else if (type == QStringLiteral("Tcl/Tk"))
        return new TclParser(view);
    else if (type == QStringLiteral("xslt"))
        return new XsltParser(view);

    // ...the last one, our dummy
    Parser *p = new DummyParser(type, view);
    // Hide the only left over general option there, so menu is deactivated
    view->m_viewSort->setVisible(false);

    return p;
}


void Parser::prepareForParse()
{
    if (!p_viewTree->isChecked()) {
        p_indexTree->setRootIsDecorated(0);
        for (int i : qAsConst(m_detachedNodeTypes)) {
            QTreeWidgetItem *node = new QTreeWidgetItem(p_indexTree, i);
            node->setText(0, p_viewOptions.value(i)->objectName());
            if (p_addIcons->isChecked()) {
            node->setIcon(0, p_icons.value(i)); }
            node->setData(0, NodeData::Line, -1);

            p_rootNodes.insert(i, node);
        }
        return;
    }

    p_indexTree->setRootIsDecorated(1);

    for(int i = 0; true; i++) {
        QAction *viewOption = p_viewOptions.value(i);
        if (!viewOption) {
            break;
        }
        QTreeWidgetItem *node = new QTreeWidgetItem(p_indexTree, i);
        node->setText(0, viewOption->objectName());
        if (p_addIcons->isChecked()) {
        node->setIcon(0, p_icons.value(i)); }
        node->setData(0, NodeData::Line, -1);

        p_rootNodes.insert(i, node);
    }
}


bool Parser::incrementLineNumber()
{
    if (p_lineNumber >= p_document->lines()) {
        return false;
    }

    ++p_lineNumber;

    return true;
}


QString Parser::rawLine(int offset /*= 0*/) const
{
    int lineNumber = p_lineNumber - 1 + offset;

    if (lineNumber >= p_document->lines()) {
        return QString();
    }
    if (lineNumber < 0) {
        return QString();
    }

    return p_document->line(lineNumber);
}


bool Parser::appendNextLine()
{
//     bool debugA = m_line.isEmpty();

    if (p_lineNumber >= p_document->lines()) {
        return false;
    }

    if (!m_line.isEmpty()) {
        m_line.append(QLatin1Char(' '));
    }

    m_line.append(p_document->line(p_lineNumber++));

//     if (debugA)
//         qDebug() << "A " << p_lineNumber << m_line.left(35);
//     else
//         qDebug() << "A+" << p_lineNumber << m_line.left(35);

    return true;
}


void Parser::parse()
{
    p_view->m_indexTree->clear();
    p_view->m_indexList.clear();
    p_view->m_filtered = false;
    p_currentLineNumber = p_view->m_currentLineNumber;

    p_lastNode = nullptr;
    p_rootNodes.clear();
    p_usefulOptions.clear();
    m_funcAtWork.clear();
    p_lineNumber = 0;

    // Make all options visible because "not visible" is treated by Qt as "not enabled"
    QList<QAction*> viewOptions(p_viewOptions.values());
    for (QAction *action : qAsConst(viewOptions)) {
        action->setVisible(true);
    }
    QHashIterator<QAction*, QAction*> modifierOptionsIterator(p_modifierOptions);
    while (modifierOptionsIterator.hasNext()) {
        modifierOptionsIterator.next();
        modifierOptionsIterator.key()->setEnabled(modifierOptionsIterator.value()->isChecked());
    }

    prepareForParse();
    parseDocument();

    if ((!p_viewExpanded->isChecked()) && (p_currentLineNumber != -1)) {
        // User edited below last added node
        QTreeWidgetItem *node = lastNode();
        while (node) {
            p_indexTree->expandItem(node);
            node = node->parent();
        }
    }

    // Keep the tree free from useless root noodes
    for (QTreeWidgetItem* node : qAsConst(p_rootNodes)) {
        if (!node->childCount()) {
            node->setHidden(true);
        }
    }

    // Keep the context menu free from useless options
    for (QAction *action : qAsConst(viewOptions)) {
        action->setVisible(p_usefulOptions.contains(action));
    }
    modifierOptionsIterator.toFront();
    while (modifierOptionsIterator.hasNext()) {
        modifierOptionsIterator.next();
        modifierOptionsIterator.key()->setVisible(modifierOptionsIterator.value()->isVisible());
    }
}


void Parser::docChanged()
{
    p_document = p_view->m_mainWindow->activeView()->document();
}


bool Parser::nodeTypeIsWanted(int nodeType)
{
    QAction *viewOption = p_viewOptions.value(nodeType, nullptr);

    if (!viewOption) {
        qDebug() << "FATAL - No option to nodeType " << nodeType;
        return false;
    }

    p_usefulOptions.insert(viewOption);

    if (!viewOption->isChecked() || !viewOption->isEnabled()) {
        return false;
    }

    return true;
}


void Parser::setNodeProperties(QTreeWidgetItem *const node, const int nodeType, const QString &text, const int lineNumber)
{
    static QTreeWidgetItem *lastNodeWasDetached = nullptr;

    if (!node) {
        return;
    }

    node->setText(0, text);
    if (p_addIcons->isChecked()) {
    node->setIcon(0, p_icons.value(nodeType)); }
    node->setData(0, NodeData::Line, lineNumber);
    node->setData(0, NodeData::EndLine, -1); // ATM, we don't know the end line

    if (p_viewExpanded->isChecked()) {
        p_indexTree->expandItem(node->parent());

    // Expand in advance the node which will be the "current" to avoid annoying flicker
    } else if (p_currentLineNumber != -1) {
        if (lineNumber == p_currentLineNumber) {
            QTreeWidgetItem *nodeToExpand = node;
            while (nodeToExpand) {
                p_indexTree->expandItem(nodeToExpand);
                nodeToExpand = nodeToExpand->parent();
            }
            p_currentLineNumber = -1; // We are done, don't try again

        } else if ((lineNumber > p_currentLineNumber) && !lastNodeWasDetached) {
            QTreeWidgetItem *nodeToExpand;
            nodeToExpand = lastNodeWasDetached ? lastNodeWasDetached : p_lastNode;
            nodeToExpand = (lineNumber == p_currentLineNumber) ? node : nodeToExpand;
            p_currentLineNumber = -1; // We are done, don't try again
            while (nodeToExpand) {
                p_indexTree->expandItem(nodeToExpand);
                nodeToExpand = nodeToExpand->parent();
            }
        }

        if ((p_currentLineNumber < 0) && (!lastNodeWasDetached) && (m_detachedNodeTypes.contains(nodeType))) {
            // To minimize flicker even more, expand in case of a just expanded
            // detached node also the last none detached node
            QTreeWidgetItem *nodeToExpand = p_lastNode;
            while (nodeToExpand) {
                p_indexTree->expandItem(nodeToExpand);
                nodeToExpand = nodeToExpand->parent();
            }
        }
    }

    if (m_detachedNodeTypes.contains(nodeType)) {
        node->setData(0, NodeData::EndLine, lineNumber);
        lastNodeWasDetached = node;
    } else {
        p_lastNode = node;
        lastNodeWasDetached = nullptr;
    }

    p_view->m_indexList.append(node);
}


QAction *Parser::addViewOption(const QString &name, const QString &caption)
{
    QAction *viewOption = p_menu.addAction(caption, p_view, &IndexView::parseDocument);
    viewOption->setCheckable(true);

    // The object name must be unique due to use as config key
    if (p_menu.findChild<QAction*>(name)) {
        QString uniqueName;
        int i = 1;
        do {
            uniqueName = name + QString::number(++i, 10);
        } while (p_menu.findChild<QAction*>(uniqueName));
        viewOption->setObjectName(uniqueName);

    } else {
        viewOption->setObjectName(name);
    }


    return viewOption;
}


QAction *Parser::registerViewOption(const int nodeType, const IconCollection::IconType iconType, const QString &name, const QString &caption)
{
    QAction *viewOption = addViewOption(name, caption);

    viewOption->setIcon(IconCollection::getIcon(iconType));

    p_icons.insert(nodeType, IconCollection::getIcon(iconType));
    p_viewOptions.insert(nodeType, viewOption);

    return viewOption;
}


QAction *Parser::registerViewOptionModifier(const int nodeType, const QString &name, const QString &caption)
{
    QAction *viewOption = addViewOption(name, caption);

    if (!p_viewOptions.contains(nodeType)) {
        qDebug() << "FATAL: View option not registred:" << nodeType << "Modifier caption:" << caption;
        qDebug() << "Hint:  Call registerViewOption() first";
        return nullptr;
    }

    viewOption->setIcon(p_icons.value(nodeType));

    p_modifierOptions.insert(viewOption, p_viewOptions.value(nodeType));

    return viewOption;
}


void Parser::addViewOptionSeparator()
{
    p_menu.addSeparator();
}


void Parser::addViewOptionDependency(int dependentNodeType, int dependencyNodeType)
{
    if (!p_viewOptions.contains(dependentNodeType)) {
        qDebug() << "FATAL: View option dependent not registred:" << dependentNodeType;
        return;
    }
    if (!p_viewOptions.contains(dependencyNodeType)) {
        qDebug() << "FATAL: View option dependency not registred:" << dependencyNodeType;
        return;
    }

    p_modifierOptions.insert(p_viewOptions.value(dependentNodeType), p_viewOptions.value(dependencyNodeType));
}

// kate: space-indent on; indent-width 4; replace-tabs on;
