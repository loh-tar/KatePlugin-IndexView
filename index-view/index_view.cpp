/*   This file is part of KatePlugin-IndexView
 *
 *   IndexView Class
 *   Copyright (C) 2018 loh.tar@googlemail.com
 *
 *   Based on
 *   plugin_katesymbolviewer
 *   Copyright (C) 2014 Kåre Särs <kare.sars@iki.fi>
 *   Copyright (C) 2003 Massimo Callegari <massimocallegari@yahoo.it>
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

#include <QEvent>
#include <QHeaderView>
#include <QKeyEvent>
#include <QToolButton>
#include <QStackedWidget>
#include <QVBoxLayout>

#include <KConfigGroup>
#include <KLocalizedString>
#include <KSharedConfig>
#include <KTextEditor/Application>
#include <KTextEditor/Editor>
#include <KTextEditor/View>
#include <KXMLGUIFactory>

#include "filter_box.h"
#include "icon_collection.h"
#include "kate_plugin_index_view.h"
#include "parser.h"

#include "index_view.h"

#define UpdateCurrItemDelay 400 // Sensible compromise, determined by try&error

IndexView::IndexView(KatePluginIndexView *plugin, KTextEditor::MainWindow *mw)
    : QObject(mw)
    , KTextEditor::SessionConfigInterface()
    , m_plugin(plugin)
    , m_mainWindow(mw)
{
    // FIXME KF5 KGlobal::locale()->insertCatalog("kateindexviewplugin");

    KXMLGUIClient::setComponentName(QStringLiteral("kateindexview"), i18n("IndexView"));
    setXMLFile(QStringLiteral("ui.rc"));

    m_mainWindow->guiFactory()->addClient(this);

    m_parseDelayTimer.setSingleShot(true);
    connect(&m_parseDelayTimer, &QTimer::timeout, this, &IndexView::parseDocument);

    m_updateCurrItemDelayTimer.setSingleShot(true);
    connect(&m_updateCurrItemDelayTimer, &QTimer::timeout, this, [this]() {
        updateCurrTreeItem();
    });

    m_filterDelayTimer.setSingleShot(true);
    connect(&m_filterDelayTimer, &QTimer::timeout, this, &IndexView::filterTree);

    m_filterBox = new FilterBox(this, plugin);
    connect(m_filterBox, &QComboBox::currentTextChanged, this, &IndexView::docSelectionChanged);

    m_toolview = m_mainWindow->createToolView(plugin, QStringLiteral("kate_plugin_indexview")
                                            , KTextEditor::MainWindow::Left
                                            , plugin->icon(), plugin->name());

    QWidget *container = new QWidget(m_toolview);
    m_mainLayout = new QVBoxLayout(container);
    m_mainLayout->setContentsMargins(0, 0, 0, 0);
    m_mainLayout->setSpacing(0);
    QAction *act = new QAction();
    connect(act, &QAction::triggered, this, &IndexView::lookupIndex);
    act->setIcon(QIcon::fromTheme(QStringLiteral("edit-find")));
    QToolButton *button = new QToolButton();
    button->setDefaultAction(act);
    QHBoxLayout *lo = new QHBoxLayout();
    lo->setContentsMargins(0, 0, 0, 0);
    lo->setSpacing(0);
    lo->addWidget(m_filterBox, 1);
    lo->addWidget(button);
    m_mainLayout->addLayout(lo);

    m_treeStack = new QStackedWidget();
    m_welcomeTree = new QTreeWidget();
    m_welcomeTree->setFocusPolicy(Qt::NoFocus);
    m_welcomeTree->setLayoutDirection(Qt::LeftToRight);
    m_welcomeTree->setHeaderLabel(i18nc("@title:column", "Welcome"));
    m_welcomeTree->setContextMenuPolicy(Qt::NoContextMenu);
    m_welcomeTree->setIndentation(10);
    m_welcomeTree->setRootIsDecorated(0);
    QTreeWidgetItem *node = nullptr;
    node = new QTreeWidgetItem(m_welcomeTree);
    node->setText(0, i18n("This is %1\nVersion %2", m_plugin->name(), m_plugin->version()));
    node->setExpanded(true);
    m_treeStack->addWidget(m_welcomeTree);
    m_mainLayout->addWidget(m_treeStack, 1);

    m_toolview->installEventFilter(this);

    // We protect with these timer/slot combinations against a mass of doc change signals when a split view is closed
    m_viewChangedDelayTimer.setSingleShot(true);
    connect(&m_viewChangedDelayTimer, &QTimer::timeout, this, [this]() {
        viewChanged();
    });
    connect(m_mainWindow, &KTextEditor::MainWindow::viewChanged, this, [this]() {
        m_viewChangedDelayTimer.start(10);
    });

    // Ensure we don't keep stuff for gone docs
    connect(KTextEditor::Editor::instance()->application(), &KTextEditor::Application::documentWillBeDeleted, this, [this](KTextEditor::Document *doc) {
        auto parser = m_cache.take(doc);
        if (parser) {
            m_treeStack->removeWidget(parser->indexTree());
            delete parser;
        }
        if (m_cache.isEmpty()) {
            m_treeStack->setCurrentWidget(m_welcomeTree);
        }
    });

    m_plugin->m_views.insert(this);
    loadViewSettings();
    m_viewChangedDelayTimer.start(10);
}


IndexView::~IndexView()
{
    m_mainWindow->guiFactory()->removeClient(this);

    for (auto parser : m_cache) {
        m_treeStack->removeWidget(parser->indexTree());
        delete parser;
    }

    delete m_toolview;

    m_plugin->m_views.remove(this);
}


void IndexView::readSessionConfig(const KConfigGroup &config)
{
    m_filterBox->addItems(config.readEntry("FilterHistory", QStringList()));
    m_filterBox->clearEditText();
    updateFilterBoxPosition(config.readEntry("FilterBoxPosition", 0));
}


void IndexView::writeSessionConfig(KConfigGroup &config)
{
    QStringList history;
    for (int i = 0; i < m_filterBox->count(); i++) {
         history << m_filterBox->itemText(i);
     }
     config.writeEntry("FilterHistory", history);
     config.writeEntry("FilterBoxPosition", filterBoxPosition());
}


int IndexView::filterBoxPosition()
{
    return m_mainLayout->direction();
}


void IndexView::updateFilterBoxPosition(int pos)
{
    pos = (pos < 2 || pos > 3) ? 2 : pos;
    m_mainLayout->setDirection(static_cast<QBoxLayout::Direction>(pos));
}


void IndexView::loadViewSettings()
{
    KConfigGroup mainGroup(KSharedConfig::openConfig(), QStringLiteral("PluginIndexView"));
    m_cozyClickExpand = mainGroup.readEntry(QStringLiteral("CozyClickExpand"), false);
    m_parseDelay = mainGroup.readEntry(QStringLiteral("ParseDelay"), 1000);
}


void IndexView::saveViewSettings()
{
    KConfigGroup mainGroup(KSharedConfig::openConfig(), QStringLiteral("PluginIndexView"));
    mainGroup.writeEntry(QStringLiteral("CozyClickExpand"), m_cozyClickExpand);
    mainGroup.writeEntry(QStringLiteral("ParseDelay"), m_parseDelay);
}


Parser* IndexView::parserOfCurrentView()
{
    KTextEditor::View *docView = m_mainWindow->activeView();
    if (!docView) {
        return nullptr;
    }

    KTextEditor::Document* doc = docView->document();
    if (!doc) {
        return nullptr;
    }

    return m_cache.value(doc, nullptr);
}

void IndexView::viewChanged()
{
    KTextEditor::View *docView = m_mainWindow->activeView();
    if (!docView) {
        return;
    }

    KTextEditor::Document* doc = docView->document();
    if (!doc) {
        return;
    }

    auto parser = m_cache.value(doc, nullptr);
    if (!parser) {
        docModeChanged(doc);
        return;
    }

    // In case there is a split screen created, ensure we got updates
    connect(docView, &KTextEditor::View::cursorPositionChanged, this, &IndexView::docCursorPositionChanged, Qt::UniqueConnection);
    connect(docView, &KTextEditor::View::selectionChanged, this, &IndexView::docSelectionChanged, Qt::UniqueConnection);

    if (parser->needsUpdate()) {
        m_parseDelayTimer.start(10);
    } else {
        filterTree();
        m_treeStack->setCurrentWidget(parser->indexTree());
    }
}


void IndexView::docModeChanged(KTextEditor::Document *doc)
{
    if (!doc) {
        return;
    }

    // We prefer to use the highlighting mode, in the hope we can this way avoid a couple of issues
    // - The highlighting mode is remembered by Kate when manually changed
    // - The file type often don't fit the file when the highlighting works
    QString newDocType = doc->highlightingMode();
    if (QLatin1String("None") == newDocType) {
        newDocType = doc->mode();
    }

    auto parser = m_cache.value(doc);

    if (parser) {
        if (newDocType == parser->docType()) {
            return;
        }

        m_cache.remove(doc);
        m_treeStack->removeWidget(parser->indexTree());
        delete parser;
    }

    KTextEditor::View *docView = m_mainWindow->activeView();
    if (!docView) {
        return;
    }

    if (docView->document() != doc) {
        // Some doc not of our current interest changed
        return;
    }

    parser = Parser::create(doc, newDocType, this);
    parser->loadSettings();

    m_treeStack->addWidget(parser->indexTree());

    m_cache.insert(doc, parser);

    connect(docView, &KTextEditor::View::cursorPositionChanged, this, &IndexView::docCursorPositionChanged, Qt::UniqueConnection);
    connect(docView, &KTextEditor::View::selectionChanged, this, &IndexView::docSelectionChanged, Qt::UniqueConnection);
    connect(doc, &KTextEditor::Document::modeChanged, this, &IndexView::docModeChanged);
    connect(doc, &KTextEditor::Document::highlightingModeChanged, this, &IndexView::docModeChanged);
    connect(doc, &KTextEditor::Document::textChanged, this, &IndexView::docEdited);
    connect(parser, &Parser::parsingDone, this, &IndexView::parsingDone);

    // Don't call parseDocument() direct, must wait a little until other stuff is done
    m_parseDelayTimer.start(10);
}


void IndexView::docEdited(KTextEditor::Document *doc)
{
    if (!doc) {
        return;
    }

    auto parser = m_cache.value(doc);

    if (parser) {
        parser->docNeedParsing();
    }

    KTextEditor::View *docView = m_mainWindow->activeView();
    if (docView && docView->document() != doc) {
        // Some doc not of our current interest has been edited
        return;
    }


    if (!m_toolview->isVisible()) {
        return;
    }

    m_updateCurrItemDelayTimer.stop(); // Avoid unneeded update
    m_parseDelayTimer.start(m_parseDelay);
}


void IndexView::docSelectionChanged()
{
    if (!m_toolview->isVisible()) {
        return;
    }

    m_filterDelayTimer.start(UpdateCurrItemDelay);
}


void IndexView::docCursorPositionChanged()
{
    if (!m_toolview->isVisible()) {
        return;
    }

    if (m_parseDelayTimer.isActive()) {
        // No need for update, will come anyway
        return;
    }

    m_updateCurrItemDelayTimer.start(UpdateCurrItemDelay);
}


void IndexView::lookupIndex()
{
    auto parser = parserOfCurrentView();
    if(!parser) {
        return;
    }

    // After using parserOfCurrentView() we are save
    auto docView = m_mainWindow->activeView();
    if (!docView->selection()) {
        return;
    }

    if (!m_lookupTree) {
        m_lookupTree = new QTreeWidget();
        m_lookupTree->setFocusPolicy(Qt::NoFocus);
        m_lookupTree->setLayoutDirection(Qt::LeftToRight);
        m_lookupTree->setHeaderLabel(i18nc("@title:column", "***  LOOKUP  ***"));
        m_lookupTree->setContextMenuPolicy(Qt::NoContextMenu);
        m_lookupTree->setIndentation(10);
        m_lookupTree->setRootIsDecorated(0);
        connect(m_lookupTree, &QTreeWidget::itemClicked, this, &IndexView::lookupItemClicked);
        m_treeStack->addWidget(m_lookupTree);
    }

    // Only pattern without space and at least three char long
    static const QRegularExpression rx(QStringLiteral("^\\S{3,}$"));

    QString pattern = docView->selectionText();
    if (!pattern.contains(rx)) {
        return;
    }

    m_lookupTree->clear();
    QTreeWidgetItem *docNode = nullptr;

    // Iterate over all our parser (aka open/used documents)...
    for (auto i = m_cache.cbegin(), end = m_cache.cend(); i != end; ++i) {
        // ...but skip the current one...
        if (i.value() == parser) {
            continue;
        }

        // ...and search in their index list
        for (const auto item : std::as_const(*i.value()->indexList())) {
            if (!item->text(0).contains(pattern, Qt::CaseSensitive)) {
                continue;
            }

            if (!docNode) {
                docNode = new QTreeWidgetItem(m_lookupTree);
                docNode->setText(0, i.key()->documentName());
                docNode->setExpanded(true);
                docNode->setIcon(0, QIcon::fromTheme(QStringLiteral("text-x-generic")));
            }

            QTreeWidgetItem *node = item->clone();
            // Storing a pointer in a QVariant need a special treatment
            node->setData(0, NodeData::EndLine, QVariant::fromValue<KTextEditor::Document*>(i.value()->document()));
            docNode->addChild(node);
        }

        docNode = nullptr;
    }

    if (m_lookupTree->topLevelItemCount() < 1) {
        m_filterBox->indicateMatch(FilterBox::NoMatch);
        return;
    }

    m_treeStack->setCurrentWidget(m_lookupTree);
}


void IndexView::lookupItemClicked(QTreeWidgetItem *it)
{
    if (!it) {
        return;
    }

    if (m_cozyClickExpand) {
        it->setExpanded(!it->isExpanded());
    }

    int line   = it->data(0, NodeData::Line).toInt();
    int column = it->data(0, NodeData::Column).toInt();
    // Retrieve a pointer from a QVariant need special treatment
    KTextEditor::Document* doc = it->data(0, NodeData::EndLine).value<KTextEditor::Document*>();

    if (!doc) {
        return;
    }

    m_mainWindow->activateView(doc);

    KTextEditor::View *docView = m_mainWindow->activeView();
    if (!docView) {
        return;
    }

    docView->setCursorPosition(KTextEditor::Cursor(line, column));
}


void IndexView::filterTree()
{
    if (!m_toolview->isVisible()) {
        return;
    }

    // After using parserOfCurrentView() we are save
    auto parser = parserOfCurrentView();
    if(!parser) {
        return;
    }

    auto docView = m_mainWindow->activeView();

    // To filter our tree is surprising complex because there seams to be no
    // suitable Qt build in for that. QTreeWidget::findItems() does not traverse
    // and delivers only toplevel matches

    // Only pattern without space and at least three char long
    static const QRegularExpression rx(QStringLiteral("^\\S{3,}$"));
    QString pattern = m_filterBox->currentText();
    if (docView->selection()) {
        pattern = docView->selectionText();
        if (!pattern.contains(rx)) {
            pattern = m_filterBox->currentText();
        }
    }

    if (pattern.isEmpty()) {
        restoreTree(parser);
        return;
    }

    // Test if something match...
    bool hit = false;
    for (QTreeWidgetItem *item : std::as_const(*parser->indexList())) {
        if (item->text(0).contains(pattern, Qt::CaseInsensitive)) {
            hit = true;
            break;
        }
    }

    if (!hit) {
        m_updateCurrItemDelayTimer.start(10);
        restoreTree(parser);
        m_filterBox->indicateMatch(FilterBox::NoMatch);
        return;
    }

    m_filterBox->indicateMatch(FilterBox::Match);
    parser->setTreeFiltered(true);
    auto indexTree = parser->indexTree();
    for (QTreeWidgetItem *item : std::as_const(*parser->indexList())) {
        if (item->text(0).contains(pattern, Qt::CaseInsensitive)) {
            while (item) {
                indexTree->expandItem(item);
                item->setHidden(false);
                item = item->parent();
            }
        } else {
            item->setHidden(true);
        }
    }

    m_updateCurrItemDelayTimer.start(10);
}


void IndexView::restoreTree(Parser *parser)
{
    if (!parser) {
        return;
    }

    m_filterBox->indicateMatch(FilterBox::Neutral);

    if (!parser->treeIsFiltered()) {
        m_updateCurrItemDelayTimer.start(10);
        return;
    }

    parser->setTreeFiltered(false);
    auto indexTree = parser->indexTree();

    if (parser->showAsTree()) {
        for (int i = 0; i < indexTree->topLevelItemCount(); i++) {
            indexTree->topLevelItem(i)->setExpanded(parser->showExpanded());
        }
    }

    for (QTreeWidgetItem *item : std::as_const(*parser->indexList())) {
        item->setHidden(false);
        item->setExpanded(parser->showExpanded());
    }

    QTreeWidgetItem *node = indexTree->currentItem();
    while (node) {
        indexTree->expandItem(node);
        node = node->parent();
    }

    m_updateCurrItemDelayTimer.start(10);
}


void IndexView::updateCurrTreeItem()
{
    if (!m_toolview->isVisible()) {
        return;
    }

    auto parser = parserOfCurrentView();
    if(!parser) {
        return;
    }

    // After using parserOfCurrentView() we are save
    auto docView = m_mainWindow->activeView();
    if (docView->selection()) {
        // When one select some text is an update of the item sometimes annoying
        return;
    }

    auto indexTree = parser->indexTree();
    m_treeStack->setCurrentWidget(indexTree);

    KTextEditor::Cursor cursorPos = docView->cursorPositionVirtual();
    QTreeWidgetItem *currItem = indexTree->currentItem();
    if (currItem) {
        if (currItem->data(0, NodeData::Line).toInt() == cursorPos.line() && currItem->data(0, NodeData::Column).toInt() <= cursorPos.column()) {
            indexTree->scrollToItem(currItem);
            return;
        }
    }

    bool newItemIsFuzzy = true;
    QTreeWidgetItem *newItem = nullptr;
    for (int i = 0; i < parser->indexList()->size(); ++i) {
        currItem = parser->indexList()->at(i);
        const int beginLine = currItem->data(0, NodeData::Line).toInt();
        const int beginColumn = currItem->data(0, NodeData::Column).toInt();
        // FIXME Some parser don't set the end line in some cases, as work around we use here begin line
        //qDebug() << currItem->data(0, NodeData::EndLine).toInt() << currItem->text(0);
        const int endLine   = currItem->data(0, NodeData::EndLine).toInt() < 0 ? beginLine : currItem->data(0, NodeData::EndLine).toInt();

        if (beginLine < 0) {
            // Ignore (bad) root items
            continue;
        }

        if (beginLine > cursorPos.line() || (cursorPos.line() == beginLine && cursorPos.column() < beginColumn) ) {
            // We are already below the cursor
            break;
        }

        if (cursorPos.line() == beginLine && cursorPos.column() >= beginColumn) {
            // We are just inside a candidate
            newItem = currItem;
            newItemIsFuzzy = false;
        } else if (endLine >= cursorPos.line() && beginLine < cursorPos.line()) {
            // We are inside a candidate
            newItem = currItem;
            newItemIsFuzzy = false;
        } else if (newItem && (currItem->parent() == newItem->parent() || currItem->parent() == newItem)) {
            // We are in a nested situation, we want the last one above the cursor but only
            // if this is not some detached node like FIXME/TODO
            newItem = currItem;
            newItemIsFuzzy = true;
        }
    }

    if (currItem == indexTree->currentItem() && newItemIsFuzzy) {
        // The situation is fuzzy, any change make nothing better
        indexTree->scrollToItem(indexTree->currentItem());
        return;
    }

    if (newItem && newItem != indexTree->currentItem()) {
        //qDebug() << "set new item from" << newItem->data(0, NodeData::Line).toInt() << "to" << newItem->data(0, NodeData::EndLine).toInt();
        indexTree->blockSignals(true);
        indexTree->setCurrentItem(newItem);
        indexTree->blockSignals(false);
    }

    indexTree->scrollToItem(indexTree->currentItem());
}


bool IndexView::eventFilter(QObject *obj, QEvent *event)
{
    KTextEditor::View *docView = m_mainWindow->activeView();

    if (docView && obj == m_toolview) { // Meh, we filter no other ATM but anyway
        if (event->type() == QEvent::KeyPress) {
            QKeyEvent *ke = static_cast<QKeyEvent*>(event);
            if ((ke->key() == Qt::Key_Escape)) {
                docView->setFocus();
                event->accept();
                return true;
            }
        } else if (event->type() == QEvent::Show) {
            updateCurrTreeItem();
            m_parseDelayTimer.start(10); // trigger parsing immediately
        }
    }

    return QObject::eventFilter(obj, event);
}


void IndexView::showContextMenu(const QPoint&)
{
    auto parser = parserOfCurrentView();
    if (parser) {
        parser->contextMenu()->popup(QCursor::pos());
    }
}


void IndexView::parseDocument()
{
    if (!m_toolview->isVisible()) {
        return;
    }

    auto parser = parserOfCurrentView();
    if (!parser) {
        return;
    }

    if (parser->isParsing()) {
        // Parse is already running, do it later
        docEdited(parser->document());
        return;
    }

    if (!parser->needsUpdate()) {
        return;
    }

    // The ultimate-special-final-director-cut flicker-avoidance :-/ Part A
    m_treeStack->setUpdatesEnabled(false);

    parser->parse();
}


void IndexView::parsingDone(Parser *parser)
{
    auto indexTree = parser->indexTree();
    connect(indexTree, &QTreeWidget::itemClicked, this, &IndexView::itemClicked);
    connect(indexTree, &QTreeWidget::customContextMenuRequested, this, &IndexView::showContextMenu);

    m_treeStack->addWidget(indexTree);
    m_treeStack->removeWidget(parser->mustyTree());
    parser->burnDownMustyTree();

    if (parser != parserOfCurrentView()) {
        // View/Doc has changed in the meanwhile
        return;
    }

    // Don't use timer here, we must do it all in one rush
    filterTree();
    m_updateCurrItemDelayTimer.stop(); // Started in filterTree(), but we don't need/want that now
    updateCurrTreeItem();

    // All updates are done, switch to the new tree now
    m_treeStack->setCurrentWidget(indexTree);

    // The ultimate-special-final-director-cut flicker-avoidance :-/ Part B
    // Why 300 is needed, who knows? 200 works almost but not full
    QTimer::singleShot(300, this, [this]() {
        m_treeStack->setUpdatesEnabled(true);
    });
}


void IndexView::itemClicked(QTreeWidgetItem *it)
{
    if (!it) {
        return;
    }

    int line   = it->data(0, NodeData::Line).toInt();
    int column = it->data(0, NodeData::Column).toInt();

    if (m_cozyClickExpand) {
        if (m_lastClickedItem == it || (line < 0)) {
            it->setExpanded(!it->isExpanded());
        }
        m_lastClickedItem = it;
    }

    KTextEditor::View *docView = m_mainWindow->activeView();
    if (!docView) {
        return;
    }

    docView->setCursorPosition(KTextEditor::Cursor(line, column));
    m_updateCurrItemDelayTimer.stop(); // Avoid unneeded update, yeah, strange but works because signal/slots are running immediately
}

// kate: space-indent on; indent-width 4; replace-tabs on;
