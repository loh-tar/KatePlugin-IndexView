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


#include <QApplication>
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


void DummyParser::addNode(const int nodeType, const QString &text)
{
    QTreeWidgetItem *node = new QTreeWidgetItem(p_indexTree, nodeType);

    // Don't fill p_usefulOptions, so option will hidden
    // QAction *viewOption = p_viewOptions.value(nodeType, nullptr);
    // p_usefulOptions.insert(viewOption);

    if (node) {
        node->setText(0, text);
        // Don't set icon, looks odd
        // node->setIcon(0, p_nodeTypes.value(nodeType).icon);
        node->setData(0, NodeData::Column, -1);
        node->setData(0, NodeData::Line, -1);
    }
}


void DummyParser::parseDocument()
{
    p_indexTree->setRootIsDecorated(0);
    addNode(InfoNode, i18n("Sorry, not supported yet!"));
    addNode(InfoNode, i18n("File type: %1", m_docType));
}


Parser::Parser(IndexView *view)
    : QObject(view)
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

    p_modifierOptions.append(DependencyPair(p_viewExpanded, p_viewTree));
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
    if (type == QStringLiteral("Bash") || type == QStringLiteral("Zsh"))
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
    p_indexTree->setRootIsDecorated(p_viewTree->isChecked());
}

QTreeWidgetItem *Parser::rootNode(int nodeType)
{
    QTreeWidgetItem *node = p_rootNodes.value(nodeType, nullptr);
    if (node) {
        return node;
    }

    if (m_detachedNodeTypes.contains(nodeType)) {
        // We want detached nodes always on top of the view, in order of their enum value
        node = new QTreeWidgetItem(nodeType);
        int index = 0;
        while (p_indexTree->topLevelItem(index) && p_indexTree->topLevelItem(index)->type() < nodeType) {
            ++index;
        }
        p_indexTree->insertTopLevelItem(index, node);

    } else {
        node = new QTreeWidgetItem(p_indexTree, nodeType);
    }

    const auto nt = p_nodeTypes.value(nodeType);

    node->setText(0, nt.name);
    if (p_addIcons->isChecked()) {
    node->setIcon(0, nt.icon);}
    node->setData(0, NodeData::Line, -1);
    p_rootNodes.insert(nodeType, node);

    return node;
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

    // Keep the editing responsive when the file is large
    if (m_runTime.hasExpired(100)) {
        qApp->processEvents();
        m_runTime.start();
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
    // Looks odd, but we need to do this here because some parser parse in some situations
    // twice, e.g. DocumentParser, and then we end up with a doubled result tree
    p_view->m_indexTree->clear();
    p_view->m_indexList.clear();
    p_view->m_filtered = false;

    p_document = p_view->m_mainWindow->activeView()->document();

    p_lastNode = nullptr;
    p_rootNodes.clear();
    p_usefulOptions.clear();
    m_funcAtWork.clear();
    p_lineNumber = 0;

    // Make all options visible because "not visible" is treated by Qt as "not enabled"
    for (auto it = p_nodeTypes.constBegin(); it != p_nodeTypes.constEnd(); ++it) {
        if (it.value().option) {
            it.value().option->setVisible(true);
        }
    }

    if (p_nesting1) {
        p_nesting1->setVisible(true);
        p_nesting2->setVisible(true);
        p_nesting3->setVisible(true);
        p_nesting4->setVisible(true);
    }

    // Enable (or not) view options with respect of their dependencies
    for (int i = 0; i < p_modifierOptions.size(); ++i) {
        p_modifierOptions.at(i).dDent->setEnabled(p_modifierOptions.at(i).dDency->isChecked() && p_modifierOptions.at(i).dDency->isEnabled());
    }

    prepareForParse();
    m_runTime.start();
    parseDocument();

    // Keep the context menu free from useless options
    for (auto it = p_nodeTypes.constBegin(); it != p_nodeTypes.constEnd(); ++it) {
        if (it.value().option) {
            it.value().option->setVisible(p_usefulOptions.contains(it.value().option));
        }
    }

    if (p_nesting1) {
        p_nesting1->setVisible(p_maxNesting > 0);

        p_nesting2->setVisible(p_maxNesting > 1);
        p_nesting2->setEnabled(p_nesting1->isChecked() && p_nesting1->isEnabled());

        p_nesting3->setVisible(p_maxNesting > 2);
        p_nesting3->setEnabled(p_nesting2->isChecked() && p_nesting2->isEnabled());

        p_nesting4->setVisible(p_maxNesting > 3);
        p_nesting4->setEnabled(p_nesting3->isChecked() && p_nesting3->isEnabled());
    }

    for (int i = 0; i < p_modifierOptions.size(); ++i) {
        p_modifierOptions.at(i).dDent->setVisible(p_modifierOptions.at(i).dDency->isVisible());
    }

    if (!p_viewTree->isChecked()) {
        // To offer a plain list of our hard-raised tree, we must hack him in pieces again :-(
        // We do it this way because the tree help us to decide what todo while parsing. And because
        // we have already a list of our nodes of interest (m_indexList), we use this list to decide
        // how to process with each node, keep or delete.

        // We cut each "main" limb off the tree until he is empty..
        while (p_indexTree->topLevelItemCount() > 0) {
            QTreeWidgetItem *item = p_indexTree->takeTopLevelItem(0);
            while (item->childCount() > 0) {
                QTreeWidgetItem *ci = item->takeChild(0);
                if (ci->isHidden()) {
                    // A hidden one is not needed and can be deleted now
                    delete ci;
                } else if (!p_view->m_indexList.contains(ci)) {
                    // Not in list, drop it. Unlikely for child nodes, but who knows
                    delete ci;
                }
            }

            if (!p_view->m_indexList.contains(item)) {
                // Not in list, drop it. Happens often for top level nodes.
                delete item;
            }
        }

        // ...and now add the (good) nodes again to our non-tree..tree widget
        for (int i = 0; i < p_view->m_indexList.size(); ++i) {
            QTreeWidgetItem *item = p_view->m_indexList.at(i);
            while (item->childCount() > 0) {
                QTreeWidgetItem *ci = item->takeChild(0);
                if (ci->isHidden()) {
                    delete ci;
                } else if (!p_view->m_indexList.contains(ci)) {
                    delete ci;
                }
            }
            // Here is the beef!
            p_indexTree->addTopLevelItem(p_view->m_indexList.at(i));
        }

        // We need to take care of p_lastNode or we can crash e.g. with DocumentParser::parse()
        if (!p_view->m_indexList.contains(p_lastNode)) {
            p_lastNode = nullptr;
        }

        p_indexTree->setRootIsDecorated(0);
    }
}


bool Parser::nodeTypeIsWanted(int nodeType)
{
    QAction *viewOption = p_nodeTypes.value(nodeType).option;

    if (!viewOption) {
        // Prior was an option mandatory, but now I think this way is better
        return true;
    }

    p_usefulOptions.insert(viewOption);

    if (!viewOption->isChecked() || !viewOption->isEnabled()) {
        return false;
    }

    return true;
}


void Parser::setNodeProperties(QTreeWidgetItem *const node, const int nodeType, const QString &text, const int lineNumber, const int columnNumber/* = 0*/)
{
    if (!node) {
        return;
    }

    node->setText(0, text);
    if (p_addIcons->isChecked()) {
    node->setIcon(0, p_nodeTypes.value(nodeType).icon); }
    node->setData(0, NodeData::Line, lineNumber);
    node->setData(0, NodeData::Column, columnNumber);
    node->setData(0, NodeData::EndLine, -1); // ATM, we don't know the end line

    if (p_viewExpanded->isChecked()) {
        p_indexTree->expandItem(node->parent());
    }

    if (m_detachedNodeTypes.contains(nodeType)) {
        node->setData(0, NodeData::EndLine, lineNumber);
    } else {
        p_lastNode = node;
    }

    p_maxNesting = qMax(p_maxNesting, p_nestingLevel);

    if (!nodeTypeIsWanted(nodeType) || (p_nestingAllowed < (p_nestingLevel + p_nestingLevelAdjustment))) {
        node->setHidden(true);
    } else {
        p_view->m_indexList.append(node);
    }
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

void Parser::useNestingOptions(bool adjust/* = false*/)
{
    p_nestingLevelAdjustment = adjust ? 1 : 0;

    p_nesting1     = addViewOption(QStringLiteral("Nesting1"), i18n("Show nesting 1"));
    p_nesting2     = addViewOption(QStringLiteral("Nesting2"), i18n("Show nesting 2"));
    p_nesting3     = addViewOption(QStringLiteral("Nesting3"), i18n("Show nesting 3"));
    p_nesting4     = addViewOption(QStringLiteral("Nesting4"), i18n("Show nesting 4+"));

    addViewOptionSeparator();
}


void Parser::resetNesting()
{
    Q_ASSERT_X(p_nesting1, "Parser::resetNesting", "No p_nesting1, forgot to call useNestingOptions() ?");

    p_nestingAllowed   = 100000; // Just some big enough value, unlikely to reach in reality..
    p_nestingLevel = -1;
    p_maxNesting =  -1;

    if (!p_nesting4->isChecked()) {
        p_nestingAllowed = 4;
    }
    if (!p_nesting3->isChecked()) {
        p_nestingAllowed = 3;
    }
    if (!p_nesting2->isChecked()) {
        p_nestingAllowed = 2;
    }
    if (!p_nesting1->isChecked()) {
        p_nestingAllowed = 1;
    }
}


void Parser::setNodeTypeIcon(const int nodeType, const int size, const int qtGlobalColorEnum/* = -1*/)
{
    if (p_nodeTypes.contains(nodeType)) {
        p_nodeTypes[nodeType].icon = IconCollection::getIcon(size, qtGlobalColorEnum);
    } else {
        p_nodeTypes.insert(nodeType, NodeTypeStruct(QString(), IconCollection::getIcon(size, qtGlobalColorEnum)));
    }
}


QAction *Parser::registerViewOption(const int nodeType, const IconCollection::IconType iconType, const QString &name, const QString &caption)
{
    QAction *viewOption = addViewOption(name, caption);

    viewOption->setIcon(IconCollection::getIcon(iconType));

//  p_nodeTypes.insert(nodeType, NodeTypeStruct(name, IconCollection::getIcon(size, qtGlobalColorEnum))); TODO
    p_nodeTypes.insert(nodeType, NodeTypeStruct(name, IconCollection::getIcon(iconType), viewOption));

    return viewOption;
}


QAction *Parser::registerViewOptionModifier(const int nodeType, const QString &name, const QString &caption)
{
    const auto nt = p_nodeTypes.value(nodeType);

    if (!nt.option) {
        qDebug() << "FATAL: View option not registered:" << nodeType << "Modifier caption:" << caption;
        qDebug() << "Hint:  Call registerViewOption() first";
        return nullptr;
    }

    QAction *viewOption = addViewOption(name, caption);
    viewOption->setIcon(nt.icon);
    p_modifierOptions.append(DependencyPair(viewOption, nt.option));

    return viewOption;
}


void Parser::addViewOptionSeparator()
{
    p_menu.addSeparator();
}


void Parser::addViewOptionDependency(int dependentNodeType, int dependencyNodeType)
{
    const auto dent = p_nodeTypes.value(dependentNodeType);
    if (!dent.option) {
        qDebug() << "FATAL: View option dependent not registered:" << dependentNodeType;
        return;
    }

    const auto dency = p_nodeTypes.value(dependencyNodeType);
    if (!dency.option) {
        qDebug() << "FATAL: View option dependency not registered:" << dependencyNodeType;
        return;
    }

    p_modifierOptions.append(DependencyPair(dent.option, dency.option));
}

// kate: space-indent on; indent-width 4; replace-tabs on;
