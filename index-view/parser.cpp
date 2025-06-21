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
#include <QHeaderView>
#include <QScrollBar>

#include <KConfigGroup>
#include <KLocalizedString>
#include <KSharedConfig>

#include "icon_collection.h"

#include "bash_parser.h"
#include "cpp_parser.h"
#include "ecma_parser.h"
#include "fortran_parser.h"
#include "go_parser.h"
#include "markdown_parser.h"
#include "perl_parser.h"
#include "php_parser.h"
#include "plaintext_parser.h"
#include "python_parser.h"
#include "ruby_parser.h"
#include "tcl_parser.h"
#include "xml_type_parser.h"

#include "parser.h"


DummyParser::DummyParser(QObject *view, KTextEditor::Document *doc)
    : Parser(view, doc)
{
    using namespace IconCollection;
    registerViewOption(InfoNode, Red1Icon, QStringLiteral("Info"), i18n("Show Info"));

    // Hide all possible general options to deactivate the menu
    p_viewSort->setVisible(false);
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
    addNode(InfoNode, i18n("File type: %1", docType()));
}


Parser::Parser(QObject *view, KTextEditor::Document *doc)
    : QObject(view)
    , p_document(doc)
    , p_indexTree(new QTreeWidget())
{
    p_viewSort     = addViewOption(QStringLiteral("SortIndex"), i18n("Show Sorted"));
    p_viewTree     = addViewOption(QStringLiteral("TreeView"), i18n("Tree View"));
    p_addIcons     = addViewOption(QStringLiteral("AddIcons"), i18n("Adorn View"));
    p_viewExpanded = addViewOption(QStringLiteral("ExpandView"), i18n("Expand View"));

    addViewOptionSeparator();

    using namespace IconCollection;
    // setNodeTypeIcon(GitConflictNode, GitConflictIcon);
    p_nodeTypes.insert(GitConflictNode, NodeTypeStruct(QStringLiteral("* GIT CONFLICT *"), IconCollection::getIcon(GitConflictIcon)));
    m_detachedNodeTypes << GitConflictNode;

    p_modifierOptions.append(DependencyPair(p_viewExpanded, p_viewTree));
}


Parser::~Parser()
{
    saveSettings();

    delete p_indexTree;
}


Parser *Parser::create(KTextEditor::Document *doc, const QString &type, QObject *view)
{
    // FIXME Do you know some more elegant way to achieve the desired result?
    // The bad things here are, that you must add here each new parser and recompile
    // at least this plugin

    const QString typeToken = QStringLiteral(";%1;").arg(type);
    // Ordered by parser class name
    static const QString typeBashParser(QStringLiteral(";Bash;Zsh;"));
    static const QString typeCppParser(QStringLiteral(";C++;C;ANSI C89;Java;Groovy;"));
    static const QString typeEcmaParser(QStringLiteral(";ActionScript 2.0;JavaScript;QML;"));
    static const QString typeIniFileParser(QStringLiteral(";INI Files;.desktop;TOML;"));
    static const QString typePlainTextParser(QStringLiteral(";Normal;.desktop;")); // This .desktop is now only some placeholder, remove it!
    static const QString typeXmlTypeParser(QStringLiteral(";DTD;XML;HTML;SGML;xslt;"));

    Parser *newParser;

    // Ordered by parser class name, except...
    if (type == QStringLiteral("AsciiDoc"))
        newParser = new AsciiDocParser(view, doc);
    else if (typeBashParser.contains(typeToken))
        newParser = new BashParser(view, doc);
    else if (typeCppParser.contains(typeToken))
        newParser = new CppParser(view, doc);
    else if (type == QStringLiteral("Diff"))
        newParser = new DiffFileParser(view, doc);
    else if (typeEcmaParser.contains(typeToken))
        newParser = new EcmaParser(view, doc);
    else if (type.startsWith(QStringLiteral("Fortran")))
        newParser = new FortranParser(view, doc);
    else if (type.startsWith(QStringLiteral("Go")))
        newParser = new GoParser(view, doc);
    else if (typeIniFileParser.contains(typeToken))
        newParser = new IniFileParser(view, doc);
    else if (type == QStringLiteral("Makefile"))
        newParser = new MakefileParser(view, doc);
    else if (type == QStringLiteral("Markdown"))
        newParser = new MarkdownParser(view, doc);
    else if (type == QStringLiteral("Perl"))
        newParser = new PerlParser(view, doc);
    else if (type == QStringLiteral("PHP (HTML)"))
        newParser = new PhpParser(view, doc);
    else if (typePlainTextParser.contains(typeToken))
        newParser = new PlainTextParser(view, doc);
    else if (type == QStringLiteral("Python"))
        newParser = new PythonParser(view, doc);
    else if (type == QStringLiteral("Ruby"))
        newParser = new RubyParser(view, doc);
    else if (type == QStringLiteral("Tcl/Tk"))
        newParser = new TclParser(view, doc);
    else if (typeXmlTypeParser.contains(typeToken))
        newParser = new XmlTypeParser(view, doc);

    // ...the last one, our dummy
    else newParser = new DummyParser(view, doc);

    newParser->finalizeSetup(type);

    return newParser;
}


void Parser::finalizeSetup(const QString &docType)
{
    p_docType = docType;

    QAction *menuHeading = new QAction(QLatin1String(metaObject()->className()));
    menuHeading->setSeparator(true);
    contextMenu()->insertAction(contextMenu()->actions().at(0), menuHeading);
}


void Parser::loadSettings()
{
    KConfigGroup mainGroup(KSharedConfig::openConfig(), QStringLiteral("PluginIndexView"));
    KConfigGroup config(&mainGroup, docType());
    // Sub-Menus are not supported here, if needed see https://stackoverflow.com/a/38429982
    for (QAction *action : contextMenu()->actions()) {
        if (action->isSeparator()) {
            continue;
        }
        action->blockSignals(true);
        if (action->objectName() == QStringLiteral("SortIndex")) {
            // SortIndex setting need sadly a lot of special treatment
            if (config.readEntry(action->objectName(), false)) {
                action->setChecked(true);
                indexTree()->setSortingEnabled(true);
                indexTree()->sortItems(0, static_cast<Qt::SortOrder>(config.readEntry(QStringLiteral("SortIndexOrder"), 0/*Qt::AscendingOrder*/)));
            } else {
                action->setChecked(false);
                indexTree()->setSortingEnabled(false);
            }
        } else {
            action->setChecked(config.readEntry(action->objectName(), true));
        }
        action->blockSignals(false);
    }
}


void Parser::saveSettings()
{
    if (!p_viewOptionsChanged) {
        return;
    }

    p_viewOptionsChanged = false; // Well, not really needed, we will die now, but anyway

    KConfigGroup mainGroup(KSharedConfig::openConfig(), QStringLiteral("PluginIndexView"));
    KConfigGroup config(&mainGroup, docType());
    for (QAction *action : contextMenu()->actions()) {
        if (action->isSeparator()) {
            continue;
        }
        config.writeEntry(action->objectName(), action->isChecked());
    }

    // SortIndex setting need special treatment
    config.writeEntry(QStringLiteral("SortIndexOrder"), static_cast<int>(indexTree()->header()->sortIndicatorOrder()));
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


QString Parser::nextLineOrBust()
{
    static const QString headTag(QStringLiteral("<<<<<<< HEAD"));
    static const QString equalsTag(QStringLiteral("======="));
    static const QRegularExpression rxShaTag(QStringLiteral(R"(^>>>>>>> (\w+) \((.+)\))"));

    QString newline = p_document->line(p_lineNumber++);
    if (!newline.startsWith(headTag)) {
        return newline;
    }

    int conflictLineNumber = p_lineNumber - 1;

    // To avoid a false detection, we make a book keeping about found "tags"
    // <<<<<<< HEAD                          was already found, next must be
    // =======                               and equalsTag and the last
    // >>>>>>> 24175c8 (bla, blub, text)     something like this
    int state = 1; // 0 = no tag, 1 = HEAD tag, 2 = equals tag, 3 = sha tag
    bool ok = true;
    QRegularExpressionMatch rxMatch;

    for (int ln = p_lineNumber; ln < p_document->lines(); ++ln) {
        newline = p_document->line(ln);

        if (newline.startsWith(headTag)) {
            conflictLineNumber = ln;
            ok = (state == 0);
            state = ok ? 1 : 0;
        } else if (newline.startsWith(equalsTag)) {
            ok = (state == 1);
            state = ok ? 2 : 0;
        } else if (newline.contains(rxShaTag, &rxMatch)) {
            ok = (state == 2);
            state = ok ? 3 : 0;
        }

        if (state == 3 && ok) {
            if (!p_gitConflict) {
                p_gitConflict = true;
                p_lastNode = nullptr;
                p_rootNodes.clear();
                p_indexList.clear();
                p_indexTree->clear();
            }
            QTreeWidgetItem *node = new QTreeWidgetItem(p_indexTree, GitConflictNode);
            setNodeProperties(node, GitConflictNode, rxMatch.captured(2), conflictLineNumber);
            node->setData(0, NodeData::EndLine, ln);
            state = 0;
        }
    }

    if (!p_gitConflict) {
        // False detection and no other full block found
        return p_document->line(p_lineNumber);
    }

    p_lineNumber = p_document->lines();

    return QString();
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

    m_line.append(nextLineOrBust());

//     if (debugA)
//         qDebug() << "A " << p_lineNumber << m_line.left(35);
//     else
//         qDebug() << "A+" << p_lineNumber << m_line.left(35);

    return true;
}


#ifndef GENERATE_REPORT
// Status Report generation can be enabled by CMake switch -DREPORT=1
// or manually here by changing the 0 to 1
#define GENERATE_REPORT 0
#endif
#if GENERATE_REPORT>0
#include <QFile>
void Parser::generateReport()
{
    QString filePath = document()->url().adjusted(QUrl::RemoveFilename).path();

    if (filePath.endsWith(QLatin1StringView("/KatePlugin-IndexView/tests/"))) {
        qDebug() << "Update Status Report for file " << filePath + document()->url().fileName();
        QFile file(filePath + QLatin1StringView("reports/") + document()->url().fileName() + QLatin1StringView(".txt"));
        if (file.open(QIODevice::WriteOnly)) {
            QTextStream stream(&file);
            stream << "Status Report" << Qt::endl
                   << "===============" << Qt::endl
                   << "Parser        : " << metaObject()->className() << Qt::endl
                   << "Parser Version: " << version() << Qt::endl
                   << "Test File     : KatePlugin-IndexView/tests/" << document()->url().fileName() << Qt::endl
                   << "File CheckSum : " << document()->checksum().toHex() << Qt::endl
                   << Qt::endl
                   << "WARNING! The CheckSum equals the file on disk! Before you commit a changed" << Qt::endl
                   << "         report, reload (F5) the Test File to be save!" << Qt::endl
                   << Qt::endl << Qt::endl
                   << "View Options" << Qt::endl
                   << "--------------" << Qt::endl
                   << "Needless to say, but CHANGES HERE affect the result THERE!" << Qt::endl
                   << "So, something should only change here if options are added or removed." << Qt::endl
                   << "In any other case adjust the view options and trigger a new parsing." << Qt::endl
                   << Qt::endl;
            for (QAction *action : contextMenu()->actions()) {
                if (action->isSeparator()) {
                    continue;
                }
                stream << qSetFieldWidth(38) << Qt::right
                       << action->text()
                       << qSetFieldWidth(0) << Qt::left
                       << QLatin1StringView(" : ") + QLatin1StringView(action->isChecked() ? "yes" : "no")
                       << Qt::endl;
            }

            stream << Qt::endl << Qt::endl << qSetFieldWidth(0) << Qt::left
                   << "List of Nodes" << Qt::endl
                   << "---------------" << Qt::endl;
            for (int i = 0; i < p_indexList.size(); ++i) {
                QTreeWidgetItem *item = p_indexList.at(i);
                stream << qSetFieldWidth(80) << Qt::center << item->text(0) << qSetFieldWidth(0) << Qt::endl;
            }

            stream << qSetFieldWidth(0) << Qt::endl << Qt::endl << Qt::left
                   << "List of Nodes with line numbers" << Qt::endl
                   << "---------------------------------" << Qt::endl
                   << qSetFieldWidth(6) << Qt::left << "Node" << qSetFieldWidth(50) << "Node-Text" << qSetFieldWidth(0) << "Line Column EndLine" << Qt::endl;
            for (int i = 0; i < p_indexList.size(); ++i) {
                QTreeWidgetItem *item = p_indexList.at(i);
                stream << qSetFieldWidth(4) << Qt::right << i << qSetFieldWidth(2) << " "
                       << qSetFieldWidth(50) << Qt::left << item->text(0)
                       << qSetFieldWidth(4) << Qt::right << item->data(0, NodeData::Line).toInt()
                       << qSetFieldWidth(5) << item->data(0, NodeData::Column).toInt()
                       << qSetFieldWidth(7) << item->data(0, NodeData::EndLine).toInt()<< Qt::endl;
            }
        }
    }
}
#else
void Parser::generateReport()
{
    // Nothing todo! Guess the compiler will remove the entire call in this case
    return;
}
#endif


void Parser::parse()
{
    if (!needsUpdate()) {
        // I'm stubborn and don't want unneeded CPU waste
        return;
    }

    if (isParsing()) {
        // Don't do stupid stuff! We may crash!
        return;
    }

    p_docNeedParsing = false;
    p_parsingIsRunning = true;

    p_gitConflict = false;
    p_mustyTree = p_indexTree;
    p_indexTree = new QTreeWidget();
    p_indexList.clear();

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
    generateReport();

    if (p_gitConflict) {
        p_indexTree->setFocusPolicy(Qt::NoFocus);
        p_indexTree->setLayoutDirection(Qt::LeftToRight);
        p_indexTree->setHeaderLabel(i18nc("@title:column", ">>>  GIT CONFLICT  <<<"));
        p_indexTree->setContextMenuPolicy(Qt::NoContextMenu);
        p_indexTree->setIndentation(10);
        p_indexTree->setRootIsDecorated(0);
        Q_EMIT parsingDone(this);
        return;
    }

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
        // we have already a list of our nodes of interest (p_indexList), we use this list to decide
        // how to process with each node, keep or delete.

        // We cut each "main" limb off the tree until he is empty..
        while (p_indexTree->topLevelItemCount() > 0) {
            QTreeWidgetItem *item = p_indexTree->takeTopLevelItem(0);
            while (item->childCount() > 0) {
                QTreeWidgetItem *ci = item->takeChild(0);
                if (ci->isHidden()) {
                    // A hidden one is not needed and can be deleted now
                    delete ci;
                } else if (!p_indexList.contains(ci)) {
                    // Not in list, drop it. Unlikely for child nodes, but who knows
                    delete ci;
                }
            }

            if (!p_indexList.contains(item)) {
                // Not in list, drop it. Happens often for top level nodes.
                delete item;
            }
        }

        // ...and now add the (good) nodes again to our non-tree..tree widget
        for (int i = 0; i < p_indexList.size(); ++i) {
            QTreeWidgetItem *item = p_indexList.at(i);
            while (item->childCount() > 0) {
                QTreeWidgetItem *ci = item->takeChild(0);
                if (ci->isHidden()) {
                    delete ci;
                } else if (!p_indexList.contains(ci)) {
                    delete ci;
                }
            }
            // Here is the beef!
            p_indexTree->addTopLevelItem(p_indexList.at(i));
        }

        // We need to take care of p_lastNode or we can crash e.g. with DocumentParser::parse()
        if (!p_indexList.contains(p_lastNode)) {
            p_lastNode = nullptr;
        }

        p_indexTree->setRootIsDecorated(0);
    }

    p_indexTree->setFocusPolicy(Qt::NoFocus);
    p_indexTree->setLayoutDirection(Qt::LeftToRight);
    p_indexTree->setHeaderLabels({i18nc("@title:column", "Index")});
    p_indexTree->setContextMenuPolicy(Qt::CustomContextMenu);
    p_indexTree->setIndentation(10);

    if (showSorted()) {
        p_indexTree->setSortingEnabled(true);
        p_indexTree->sortItems(0, p_mustyTree->header()->sortIndicatorOrder());
    }


    Q_EMIT parsingDone(this);
}


void Parser::burnDownMustyTree()
{
    if (!isParsing()) {
        return;
    }

    // Restore scroll position from old tree to avoid flicker...
    p_indexTree->verticalScrollBar()->setSliderPosition(p_mustyTree->verticalScrollBar()->sliderPosition());
    // ...but ensure current item is visible
    p_indexTree->scrollToItem(p_indexTree->currentItem());

    delete p_mustyTree;
    p_parsingIsRunning = false;
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

    // The size of 45 is choosen to fit into the Status Report (sure could be changed..)
    // but I think no one need such long lines (ok, maybe these who want parameter to see..)
    if (text.size() > 45) {
        node->setText(0, text.first(45) + QStringLiteral("…"));
    } else {
        node->setText(0, text);
    }

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
        p_indexList.append(node);
    }
}


void Parser:: menuActionTriggered()
{
    docNeedParsing();
    // This call may a little fishy, hm...proper may to emit a signal that we need an update
    parse();
    p_viewOptionsChanged = true;
}


QAction *Parser::addViewOption(const QString &name, const QString &caption)
{
    QAction *viewOption = p_menu.addAction(caption, this, &Parser::menuActionTriggered);
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


void Parser::setNodeTypeIcon(const int nodeType, const int size, const int qtGlobalColorEnum)
{
    if (p_nodeTypes.contains(nodeType)) {
        p_nodeTypes[nodeType].icon = IconCollection::getIcon(size, qtGlobalColorEnum);
    } else {
        p_nodeTypes.insert(nodeType, NodeTypeStruct(QString(), IconCollection::getIcon(size, qtGlobalColorEnum)));
    }
}


void Parser::setNodeTypeIcon(const int nodeType, const IconCollection::IconType iconType)
{
    if (p_nodeTypes.contains(nodeType)) {
        p_nodeTypes[nodeType].icon = IconCollection::getIcon(iconType);
    } else {
        p_nodeTypes.insert(nodeType, NodeTypeStruct(QString(), IconCollection::getIcon(iconType)));
    }
}


QAction *Parser::registerViewOption(const int nodeType, const IconCollection::IconType iconType, const QString &name, const QString &caption)
{
    QAction *viewOption = addViewOption(name, caption);

    viewOption->setIcon(IconCollection::getIcon(iconType));

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
