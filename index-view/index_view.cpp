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
#include <QScrollBar>
#include <QVBoxLayout>

#include <KConfigGroup>
#include <KLocalizedString>
#include <KSharedConfig>
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
    connect(&m_updateCurrItemDelayTimer, &QTimer::timeout, this, &IndexView::updateCurrTreeItem);

    m_filterDelayTimer.setSingleShot(true);
    connect(&m_filterDelayTimer, &QTimer::timeout, this, &IndexView::filterTree);

    m_indexTree = new QTreeWidget();
    m_indexTree->setFocusPolicy(Qt::NoFocus);
    m_indexTree->setLayoutDirection(Qt::LeftToRight);
    m_indexTree->setHeaderLabels({i18nc("@title:column", "Index")});
    m_indexTree->setContextMenuPolicy(Qt::CustomContextMenu);
    m_indexTree->setIndentation(10);
    connect(m_indexTree, &QTreeWidget::currentItemChanged, this, &IndexView::currentItemChanged);
    connect(m_indexTree, &QTreeWidget::itemClicked, this, &IndexView::itemClicked);
    connect(m_indexTree, &QTreeWidget::customContextMenuRequested, this, &IndexView::showContextMenu);

    m_filterBox = new FilterBox(this, plugin);
    connect(m_filterBox, &QComboBox::currentTextChanged, this, &IndexView::docSelectionChanged);

    m_toolview = m_mainWindow->createToolView(plugin, QStringLiteral("kate_plugin_indexview")
                                            , KTextEditor::MainWindow::Left
                                            , plugin->icon(), plugin->name());


    QWidget *container = new QWidget(m_toolview);
    QVBoxLayout *layout = new QVBoxLayout(container);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout->addWidget(m_filterBox);
    layout->addWidget(m_indexTree, 1);

    // When the current file is BIG and our time-slice parsing take effect, will on very first show
    // with this delayed connect&update an ugly black QTreeWidget avoided
    QTimer::singleShot(100, this, [this]() {
        connect(m_mainWindow, &KTextEditor::MainWindow::viewChanged, this, &IndexView::docChanged);
        m_toolview->installEventFilter(this);
        docChanged();
    });

    m_plugin->m_views.insert(this);

}


IndexView::~IndexView()
{
    saveViewSettings();

    m_mainWindow->guiFactory()->removeClient(this);

    delete m_toolview;

    if (m_parser) {
        delete m_parser;
    }

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
    if (!m_parser) {
        return;
    }
    KConfigGroup mainGroup(KSharedConfig::openConfig(), QStringLiteral("PluginIndexView"));
    m_cozyClickExpand = mainGroup.readEntry(QStringLiteral("CozyClickExpand"), false);
    m_parseDelay = mainGroup.readEntry(QStringLiteral("ParseDelay"), 1000);

    KConfigGroup config(&mainGroup, m_docType);
    // Sub-Menus are not supported here, if needed see https://stackoverflow.com/a/38429982
    for (QAction *action : m_parser->contextMenu()->actions()) {
        if (action->isSeparator()) {
            continue;
        }
        action->blockSignals(true);
        if (action->objectName() == QStringLiteral("SortIndex")) {
            // SortIndex setting need sadly a lot of special treatment
            if (config.readEntry(action->objectName(), false)) {
                action->setChecked(true);
                m_indexTree->setSortingEnabled(true);
                m_indexTree->sortItems(0, static_cast<Qt::SortOrder>(config.readEntry(QStringLiteral("SortIndexOrder"), 0/*Qt::AscendingOrder*/)));
            } else {
                action->setChecked(false);
                m_indexTree->setSortingEnabled(false);
            }
        } else {
            action->setChecked(config.readEntry(action->objectName(), true));
        }
        action->blockSignals(false);
    }
}


void IndexView::saveViewSettings()
{
    if (!m_parser) {
        return;
    }
    KConfigGroup mainGroup(KSharedConfig::openConfig(), QStringLiteral("PluginIndexView"));
    mainGroup.writeEntry(QStringLiteral("CozyClickExpand"), m_cozyClickExpand);
    mainGroup.writeEntry(QStringLiteral("ParseDelay"), m_parseDelay);

    KConfigGroup config(&mainGroup, m_docType);
    for (QAction *action : m_parser->contextMenu()->actions()) {
        if (action->isSeparator()) {
            continue;
        }
        config.writeEntry(action->objectName(), action->isChecked());
    }

    // SortIndex setting need special treatment
    config.writeEntry(QStringLiteral("SortIndexOrder"), static_cast<int>(m_indexTree->header()->sortIndicatorOrder()));
}


void IndexView::docChanged()
{
    KTextEditor::View *view = m_mainWindow->activeView();
    if (!view) {
        return;
    }

    if (!view->document()) {
        return;
    }

    connect(view, &KTextEditor::View::cursorPositionChanged
          , this, &IndexView::docCursorPositionChanged, Qt::UniqueConnection);

    connect(view, &KTextEditor::View::selectionChanged
          , this, &IndexView::docSelectionChanged, Qt::UniqueConnection);

    connect(view->document(), &KTextEditor::Document::modeChanged
          , this, &IndexView::docModeChanged, Qt::UniqueConnection);

    connect(view->document(), &KTextEditor::Document::textChanged
          , this, &IndexView::docEdited, Qt::UniqueConnection);

    // No need to switch the parser when we have a split view situation
    if (m_parser && m_parser->isUsingDocument(view->document())) {
        // We don't call docCursorPositionChanged(), the delay looks bad and we
        // don't call updateCurrTreeItem() direct, must wait until new cursor position is updated
        m_updateCurrItemDelayTimer.start(0);
        return;
    }

    if (!docModeChanged()) {
        m_indexTree->setUpdatesEnabled(false);
        m_indexTree->clear(); // Hint parseDocument() not to restore scroll position
        // Don't call parseDocument() direct, must wait until new cursor position is updated
        m_parseDelayTimer.start(10);
    }
}


bool IndexView::docModeChanged()
{
    const QString newDocType = m_mainWindow->activeView()->document()->mode();

    if (newDocType == m_docType) {
        return false;
    }

    saveViewSettings();

    if (m_parser) {
        delete m_parser;
    }

    m_docType = newDocType;
    m_parser = Parser::create(m_docType, this);
    loadViewSettings();

    // This check avoid these ugly black widget on startup
    if (m_indexTree->topLevelItemCount()) {
        m_indexTree->setUpdatesEnabled(false);
        m_indexTree->clear(); // Hint parseDocument() not to restore scroll position
    }

    // Don't call parseDocument() direct, must wait until new cursor position is updated
    m_parseDelayTimer.start(10);

    return true;
}


void IndexView::docEdited()
{
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
        pattern = m_filterBox->currentText();
        if (pattern.isEmpty()) {
            restoreTree();
            return;
        }
    }

    // Test if something match...
    bool hit = false;
    for (QTreeWidgetItem *item : qAsConst(m_indexList)) {
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
    m_filtered = true;

    for (QTreeWidgetItem *item : qAsConst(m_indexList)) {
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

    if (!m_filtered) {
        return;
    }

    m_filtered = false;

    if (m_viewTree->isChecked()) {
        for (int i = 0; i < m_indexTree->topLevelItemCount(); i++) {
            m_indexTree->topLevelItem(i)->setExpanded(m_viewExpanded->isChecked());
        }
    }

    for (QTreeWidgetItem *item : qAsConst(m_indexList)) {
        item->setHidden(false);
        item->setExpanded(m_viewExpanded->isChecked());
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
    if (obj == m_toolview) { // Meh, we filter no other ATM but anyway
        if (event->type() == QEvent::KeyPress) {
            QKeyEvent *ke = static_cast<QKeyEvent*>(event);
            if ((ke->key() == Qt::Key_Escape)) {
                m_mainWindow->activeView()->setFocus();
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

    if (!m_indexTree) {
        return;
    }

    if (m_parser && m_parser->isParsing()) {
        // Parse is already running, do it later
        docEdited();
        return;
    }

    // Qt docu recommends to populate view with disabled sorting
    // https://doc.qt.io/qt-5/qtreeview.html#sortingEnabled-prop
    Qt::SortOrder sortOrder = m_indexTree->header()->sortIndicatorOrder();
    m_indexTree->setSortingEnabled(false);

    if (!m_mainWindow->activeView()) {
        m_indexTree->clear();
        return;
    }

    // To avoid unpleasant fidgeting in the tree remember current scroll position
    int scrollPosition = -1;
    if (m_indexTree->topLevelItemCount() > 0) {
        scrollPosition = m_indexTree->verticalScrollBar()->sliderPosition();
    }

    // Since we parse in time slices, we don't want intermediate updates of the tree
    // furthermore help this to avoid flicker, see below
    m_indexTree->setUpdatesEnabled(false);

    if (m_parser) {
        m_parser->parse();
    }

    if (m_viewSort->isChecked()) {
        m_indexTree->setSortingEnabled(true);
        m_indexTree->sortItems(0, sortOrder);
    }

    updateCurrTreeItem();

    if (scrollPosition > -1) {
        // Now, that all updates are done, scroll back to old position...
        m_indexTree->verticalScrollBar()->setSliderPosition(scrollPosition);
        // ...but ensure it is visible e.g. in case of some rename
        m_indexTree->scrollToItem(m_indexTree->currentItem());
    }

    filterTree();

    // This is (hopefully!) the ultimate-special-final-director-cut flicker-avoidance :-/
    // Can't say why 300, this has worked here
    QTimer::singleShot(300, this, [this]() {
        // We are done, don't forget to enable again
        m_indexTree->setUpdatesEnabled(true);
    });
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
