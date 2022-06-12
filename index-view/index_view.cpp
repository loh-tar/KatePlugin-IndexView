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

#define UpdateCurrItemDelay 200

IndexView::IndexView(KatePluginIndexView *plugin, KTextEditor::MainWindow *mw)
    : QObject(mw)
    , KTextEditor::SessionConfigInterface()
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
    QStringList titles;
    // Only column 0 "Index" will be visible, no need to translate line number titles
    titles << i18nc("@title:column", "Index") << QStringLiteral("Begin") << QStringLiteral("End");
    m_indexTree->setColumnCount(3);
    m_indexTree->setHeaderLabels(titles);
    m_indexTree->setColumnHidden(1, true);
    m_indexTree->setColumnHidden(2, true);
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

    m_toolview->installEventFilter(this);
    connect(m_mainWindow, &KTextEditor::MainWindow::viewChanged, this, &IndexView::docChanged);

    QWidget *container = new QWidget(m_toolview);
    QVBoxLayout *layout = new QVBoxLayout(container);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout->addWidget(m_filterBox);
    layout->addWidget(m_indexTree, 1);

    docChanged();
}


IndexView::~IndexView()
{
    saveViewSettings();

    m_mainWindow->guiFactory()->removeClient(this);

    delete m_toolview;

    if (m_parser) {
        delete m_parser;
    }
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

    if (!docModeChanged()) {
        m_parser->docChanged();
        // Don't call parseDocument() direct, must wait until new cursor position is updated
        QTimer::singleShot(/*UpdateCurrItemDelay + 10*/0, this, &IndexView::parseDocument);
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
    // Don't call parseDocument() direct, must wait until new cursor position is updated
    QTimer::singleShot(/*UpdateCurrItemDelay + 10*/0, this, &IndexView::parseDocument);

    return true;
}


void IndexView::docEdited()
{
    m_updateCurrItemDelayTimer.stop(); // Avoid unneeded update
    m_parseDelayTimer.start(m_parseDelay);
}


void IndexView::docSelectionChanged()
{
    m_filterDelayTimer.start(UpdateCurrItemDelay + 300);
}


void IndexView::docCursorPositionChanged()
{
    if (m_parseDelayTimer.isActive()) {
        // No need for update, will come anyway
        return;
    }
    m_updateCurrItemDelayTimer.start(UpdateCurrItemDelay);
}


void IndexView::filterTree()
{
    // To filter our tree is surprising complex because there seams to be no
    // suitable Qt build in for that. QTreeWidget::findItems() does not traverse
    // and delivers only toplevel matches
    KTextEditor::View *view = m_mainWindow->activeView();

    QString pattern;
    if (view->selection()) {
        pattern = view->selectionText();
        // Only pattern without space and at least three char long
        if (!pattern.contains(QRegularExpression(QStringLiteral("^\\S{3,}$")))) {
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
    if (!m_mainWindow) {
        return;
    }

    KTextEditor::View *editView = m_mainWindow->activeView();
    if (!editView) {
        return;
    }

    int currLine = editView->cursorPositionVirtual().line();
    if (currLine == m_currentLineNumber) {
        // Nothing to do
        return;
    }
    m_currentLineNumber = currLine;

    QTreeWidgetItem *newItem = nullptr;
    QTreeWidgetItem *fallBackItem = nullptr;
    int i = 0;
    while (i < m_indexList.size()) {
        newItem = m_indexList.at(i);
        const int beginLine = newItem->data(1, Qt::DisplayRole).toInt();
        const int endLine   = newItem->data(2, Qt::DisplayRole).toInt();
        if (beginLine == currLine) {
            break;
        }
        if (beginLine > currLine) {
            newItem = fallBackItem;// ? fallBackItem : m_indexList.at(--i);
            break;
        }
        if (endLine >= currLine) {
            fallBackItem = newItem;
        }
        ++i;
    }

    if (!newItem) {
        while (--i >= 0) {
            newItem = m_indexList.at(i);
            const int beginLine = newItem->data(1, Qt::DisplayRole).toInt();
            const int endLine   = newItem->data(2, Qt::DisplayRole).toInt();
//             if (endLine >= currLine) {
            if (beginLine != endLine) {
                break;
            }
        }
    }

    if (newItem) {
        m_indexTree->blockSignals(true);
        m_indexTree->setCurrentItem(newItem);
        m_indexTree->scrollToItem(newItem);
        m_indexTree->blockSignals(false);
    }
}

// Disabled because I can't see any effect
// bool IndexView::eventFilter(QObject *obj, QEvent *event)
// {
//     if (event->type() == QEvent::KeyPress) {
//         qDebug() << "eventFilter";
//         QKeyEvent *ke = static_cast<QKeyEvent*>(event);
//         if ((obj == m_toolview) && (ke->key() == Qt::Key_Escape)) {
//             m_mainWindow->activeView()->setFocus();
//             event->accept();
//             return true;
//         }
//     }
//
//     return QObject::eventFilter(obj, event);
// }


void IndexView::showContextMenu(const QPoint&)
{
    if (m_parser) {
        m_parser->contextMenu()->popup(QCursor::pos());
    }
}


void IndexView::parseDocument()
{
    if (!m_indexTree) {
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

    // Ensure to expand in advance the right node
    m_currentLineNumber = m_mainWindow->activeView()->cursorPositionVirtual().line();

    if (m_parser) {
        m_parser->parse();
    }

    if (m_viewSort->isChecked()) {
        m_indexTree->setSortingEnabled(true);
        m_indexTree->sortItems(0, sortOrder);
    }

    m_currentLineNumber = -1; // Ensure to bypass check in updateCurrTreeItem()
    updateCurrTreeItem();
    filterTree();
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

    int line = it->text(1).toInt(nullptr, 10);
    kv->setCursorPosition(KTextEditor::Cursor(line, 0));

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
