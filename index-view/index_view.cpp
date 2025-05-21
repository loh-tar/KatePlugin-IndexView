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

#define UpdateCurrItemDelay 400

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
        // We do this here once to avoid so much special cases in updateCurrTreeItem()
        // where we have to call it
        m_treeStack->setCurrentWidget(m_indexTree);
    });

    m_filterDelayTimer.setSingleShot(true);
    connect(&m_filterDelayTimer, &QTimer::timeout, this, &IndexView::filterTree);

    m_filterBox = new FilterBox(this, plugin);
    connect(m_filterBox, &QComboBox::currentTextChanged, this, &IndexView::docSelectionChanged);

    m_toolview = m_mainWindow->createToolView(plugin, QStringLiteral("kate_plugin_indexview")
                                            , KTextEditor::MainWindow::Left
                                            , plugin->icon(), plugin->name());

    QWidget *container = new QWidget(m_toolview);
    QVBoxLayout *layout = new QVBoxLayout(container);
    m_treeStack = new QStackedWidget();
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout->addWidget(m_filterBox);
    layout->addWidget(m_treeStack, 1);

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
    return qobject_cast<QVBoxLayout*>(m_filterBox->parentWidget()->layout())->indexOf(m_filterBox);
}


void IndexView::updateFilterBoxPosition(int pos)
{
    if (filterBoxPosition() == pos) {
        return;
    }

    // Thanks to https://stackoverflow.com/a/22012253
    QVBoxLayout *lo = qobject_cast<QVBoxLayout*>(m_filterBox->parentWidget()->layout());
    lo->removeWidget(m_filterBox);
    lo->insertWidget(pos, m_filterBox);
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


void IndexView::loadParserSettings(Parser *parser)
{
    if (!parser) {
        return;
    }

    KConfigGroup mainGroup(KSharedConfig::openConfig(), QStringLiteral("PluginIndexView"));
    KConfigGroup config(&mainGroup, parser->docType());
    // Sub-Menus are not supported here, if needed see https://stackoverflow.com/a/38429982
    for (QAction *action : parser->contextMenu()->actions()) {
        if (action->isSeparator()) {
            continue;
        }
        action->blockSignals(true);
        if (action->objectName() == QStringLiteral("SortIndex")) {
            // SortIndex setting need sadly a lot of special treatment
            if (config.readEntry(action->objectName(), false)) {
                action->setChecked(true);
                parser->indexTree()->setSortingEnabled(true);
                parser->indexTree()->sortItems(0, static_cast<Qt::SortOrder>(config.readEntry(QStringLiteral("SortIndexOrder"), 0/*Qt::AscendingOrder*/)));
            } else {
                action->setChecked(false);
                parser->indexTree()->setSortingEnabled(false);
            }
        } else {
            action->setChecked(config.readEntry(action->objectName(), true));
        }
        action->blockSignals(false);
    }
}


void IndexView::saveParserSettings(Parser *parser)
{
    if (!parser) {
        return;
    }

    KConfigGroup mainGroup(KSharedConfig::openConfig(), QStringLiteral("PluginIndexView"));
    KConfigGroup config(&mainGroup, parser->docType());
    for (QAction *action : parser->contextMenu()->actions()) {
        if (action->isSeparator()) {
            continue;
        }
        config.writeEntry(action->objectName(), action->isChecked());
    }

    // SortIndex setting need special treatment
    config.writeEntry(QStringLiteral("SortIndexOrder"), static_cast<int>(parser->indexTree()->header()->sortIndicatorOrder()));
}


void IndexView::viewChanged()
{
    KTextEditor::View *view = m_mainWindow->activeView();
    if (!view) {
        return;
    }

    KTextEditor::Document* doc = view->document();
    if (!doc) {
        return;
    }

    if (m_parser && m_parser->document() == doc) {
        connect(view, &KTextEditor::View::cursorPositionChanged, this, &IndexView::docCursorPositionChanged, Qt::UniqueConnection);
        connect(view, &KTextEditor::View::selectionChanged, this, &IndexView::docSelectionChanged, Qt::UniqueConnection);
        m_updateCurrItemDelayTimer.start(0);
        return;
    }

    m_parser = m_cache.value(doc);

    if (!m_parser) {
        docModeChanged(doc);
        return;
    }

    m_indexTree = m_parser->indexTree();
    m_indexList = m_parser->indexList();
    if (m_parser->needsUpdate()) {
        m_parseDelayTimer.start(0);
    } else {
        m_updateCurrItemDelayTimer.start(0);
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

    KTextEditor::View *view = m_mainWindow->activeView();
    if (!view) {
        return;
    }

    if (view->document() != doc) {
        // Some doc not of our current interest changed
        return;
    }

    m_parser = Parser::create(newDocType, this);
    m_indexTree = m_parser->indexTree();
    m_indexList = m_parser->indexList();
    m_treeStack->addWidget(m_indexTree);

    m_cache.insert(doc, m_parser);

    loadParserSettings(m_parser);

    connect(view, &KTextEditor::View::cursorPositionChanged, this, &IndexView::docCursorPositionChanged, Qt::UniqueConnection);
    connect(view, &KTextEditor::View::selectionChanged, this, &IndexView::docSelectionChanged, Qt::UniqueConnection);
    connect(doc, &KTextEditor::Document::modeChanged, this, &IndexView::docModeChanged);
    connect(doc, &KTextEditor::Document::highlightingModeChanged, this, &IndexView::docModeChanged);
    connect(doc, &KTextEditor::Document::textChanged, this, &IndexView::docEdited);
    connect(m_parser, &Parser::parsingDone, this, &IndexView::parsingDone);

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

    KTextEditor::View *view = m_mainWindow->activeView();
    if (view && view->document() != doc) {
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

    m_filterDelayTimer.start(UpdateCurrItemDelay + 300);
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

    if (m_updateCurrItemDelayTimer.remainingTime() == 0) {
        // Timer was very likely started with zero time e.g. after view change, don't restart now!
        return;
    }

    m_updateCurrItemDelayTimer.start(UpdateCurrItemDelay);
}


void IndexView::filterTree()
{
    if (!m_toolview->isVisible()) {
        return;
    }

    KTextEditor::View *view = m_mainWindow->activeView();
    if (!view) {
        return;
    }

    // To filter our tree is surprising complex because there seams to be no
    // suitable Qt build in for that. QTreeWidget::findItems() does not traverse
    // and delivers only toplevel matches

    // Only pattern without space and at least three char long
    static const QRegularExpression rx(QStringLiteral("^\\S{3,}$"));
    QString pattern;
    if (view->selection()) {
        pattern = view->selectionText();
        if (!pattern.contains(rx)) {
            // Without a reasonable pattern ensure the tree is shown unfiltered
            restoreTree();
            return;
        }

    } else {
        // Since we don't update the item on selection, ensure the item fit now that the selection is gone
        m_updateCurrItemDelayTimer.start(UpdateCurrItemDelay);

        pattern = m_filterBox->currentText();
        if (pattern.isEmpty()) {
            restoreTree();
            return;
        }
    }

    // Test if something match...
    bool hit = false;
    for (QTreeWidgetItem *item : std::as_const(m_indexList)) {
        if (item->text(0).contains(pattern, Qt::CaseInsensitive)) {
            hit = true;
            break;
        }
    }


    if (!hit) {
        restoreTree();
        m_filterBox->indicateMatch(FilterBox::NoMatch);
        return;
    }

    m_filterBox->indicateMatch(FilterBox::Match);
    m_parser->treeIsFiltered(true);

    for (QTreeWidgetItem *item : std::as_const(m_indexList)) {
        if (item->text(0).contains(pattern, Qt::CaseInsensitive)) {
            while (item) {
                m_indexTree->expandItem(item);
                item->setHidden(false);
                item = item->parent();
            }
        } else {
            item->setHidden(true);
        }
    }
}


void IndexView::restoreTree()
{
    m_filterBox->indicateMatch(FilterBox::Neutral);

    if (!m_parser->isTreeFiltered()) {
        return;
    }

    m_parser->treeIsFiltered(false);

    if (m_parser->showAsTree()) {
        for (int i = 0; i < m_indexTree->topLevelItemCount(); i++) {
            m_indexTree->topLevelItem(i)->setExpanded(m_parser->showExpanded());
        }
    }

    for (QTreeWidgetItem *item : std::as_const(m_indexList)) {
        item->setHidden(false);
        item->setExpanded(m_parser->showExpanded());
    }

    QTreeWidgetItem *node = m_indexTree->currentItem();
    while (node) {
        m_indexTree->expandItem(node);
        node = node->parent();
    }

    m_indexTree->scrollToItem(m_indexTree->currentItem());
}


void IndexView::updateCurrTreeItem()
{
    if (!m_toolview->isVisible()) {
        return;
    }

    if (!m_indexList.size()) {
        return;
    }

    KTextEditor::View *editView = m_mainWindow->activeView();
    if (!editView) {
        return;
    }

    if (editView->selection()) {
        // When one select some text is an update of the item sometimes annoying
        return;
    }

    KTextEditor::Cursor cursorPos = editView->cursorPositionVirtual();
    QTreeWidgetItem *currItem = m_indexTree->currentItem();
    if (currItem) {
        if (currItem->data(0, NodeData::Line).toInt() == cursorPos.line() && currItem->data(0, NodeData::Column).toInt() <= cursorPos.column()) {
            return;
        }
    }

    bool newItemIsFuzzy = true;
    QTreeWidgetItem *newItem = nullptr;
    for (int i = 0; i < m_indexList.size(); ++i) {
        currItem = m_indexList.at(i);
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

    if (currItem == m_indexTree->currentItem() && newItemIsFuzzy) {
        // The situation is fuzzy, any change make nothing better
        return;
    }

    if (newItem && newItem != m_indexTree->currentItem()) {
        //qDebug() << "set new item from" << newItem->data(0, NodeData::Line).toInt() << "to" << newItem->data(0, NodeData::EndLine).toInt();
        m_indexTree->blockSignals(true);
        m_indexTree->setCurrentItem(newItem);
        m_indexTree->scrollToItem(newItem);
        m_indexTree->blockSignals(false);
    }
}


bool IndexView::eventFilter(QObject *obj, QEvent *event)
{
    KTextEditor::View *view = m_mainWindow->activeView();

    if (view && obj == m_toolview) { // Meh, we filter no other ATM but anyway
        if (event->type() == QEvent::KeyPress) {
            QKeyEvent *ke = static_cast<QKeyEvent*>(event);
            if ((ke->key() == Qt::Key_Escape)) {
                view->setFocus();
                event->accept();
                return true;
            }
        } else if (event->type() == QEvent::Show) {
            m_parseDelayTimer.start(0); // trigger parsing immediately
        }
    }

    return QObject::eventFilter(obj, event);
}


void IndexView::showContextMenu(const QPoint&)
{
    if (m_parser) {
        m_parser->contextMenu()->popup(QCursor::pos());
    }
}


void IndexView::parseDocument()
{
    if (!m_toolview->isVisible()) {
        return;
    }

    if (!m_parser) {
        return;
    }

    if (m_parser->isParsing()) {
        // Parse is already running, do it later
        docEdited(m_parser->document());
        return;
    }

    if (!m_parser->needsUpdate()) {
        return;
    }

    m_parser->parse();
}


void IndexView::parsingDone(Parser *parser)
{
    if (parser != m_parser) {
        // View/Doc has changed in the meanwhile
        // Remove the old stuff
        m_treeStack->addWidget(parser->indexTree());
        m_treeStack->removeWidget(parser->mustyTree());
        parser->burnDownMustyTree();
        return;
    }

    m_indexTree = m_parser->indexTree();
    m_indexList = m_parser->indexList();

    // Don't use timer here, we must do it all in one rush
    filterTree();
    m_updateCurrItemDelayTimer.stop(); // Started in filterTree(), but we don't need/want that now
    updateCurrTreeItem();

    // All updates are done, switch to the new tree now...
    m_treeStack->addWidget(m_indexTree);
    m_treeStack->setCurrentWidget(m_indexTree);
    m_treeStack->removeWidget(m_parser->mustyTree());
    // ...and Parser take care to restore scroll position, so we have no flicker
    m_parser->burnDownMustyTree();
}


void IndexView::currentItemChanged(QTreeWidgetItem */*current*/, QTreeWidgetItem */*previous*/)
{
    m_currentItemChanged = true;
}


void IndexView::itemClicked(QTreeWidgetItem *it)
{
    KTextEditor::View *kv = m_mainWindow->activeView();

    // be sure we really have a view !
    if (!kv)
        return;

    int line   = it->data(0, NodeData::Line).toInt();
    int column = it->data(0, NodeData::Column).toInt();

    kv->setCursorPosition(KTextEditor::Cursor(line, column));
    m_updateCurrItemDelayTimer.stop(); // Avoid unneeded update, yeah, strange but works because signal/slots are running immediately

    if (m_currentItemChanged) {
        m_currentItemChanged = false;
        if (line >= 0) {
            return;
        }
    }

    if (m_cozyClickExpand) {
        it->setExpanded(!it->isExpanded());
    }

}

// kate: space-indent on; indent-width 4; replace-tabs on;
