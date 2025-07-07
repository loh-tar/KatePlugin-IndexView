/*   This file is part of KatePlugin-IndexView
 *
 *   KatePluginIndexView Class
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


#include <QGroupBox>
#include <QLabel>
#include <QVBoxLayout>

#include <KPluginFactory>

#include "icon_collection.h"
#include "index_view.h"
#include "parser.h"

#include "kate_plugin_index_view.h"


#define IndexViewVersion QStringLiteral("0.9.5, Jul 2025")


KatePluginIndexViewConfigPage::KatePluginIndexViewConfigPage(KatePluginIndexView *plugin, QWidget *parentWidget /*= 0L*/)
    : KTextEditor::ConfigPage(parentWidget)
    , m_plugin(plugin)
{
    setupUi(this);
    ui_pluginVersion->setText(IndexViewVersion);
    ui_pluginAuthor->setText(QStringLiteral("2018, 2022, 2024, 2025 loh.tar \n\nBased on SymbolViewer \n2014 Kåre Särs \n2003 Massimo Callegari"));
    ui_aboutPluginBox->setTitle(i18n("About %1", name()));

    connect(ui_cozyClickExpand, &QCheckBox::toggled, this, &KatePluginIndexViewConfigPage::changed);
    // QSpinBox::valueChanged need special treatment, Qt docu is not clear to me, so thanks to https://forum.qt.io/post/345501
    connect(ui_parseDelay, static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &KatePluginIndexViewConfigPage::changed);
}


KatePluginIndexViewConfigPage::~KatePluginIndexViewConfigPage()
{}


QString KatePluginIndexViewConfigPage::name() const
{
    return m_plugin->name();
}


QString KatePluginIndexViewConfigPage::fullName() const
{
    return i18n("%1 Configuration Page", m_plugin->name());
}


QIcon KatePluginIndexViewConfigPage::icon() const
{
    return m_plugin->icon();
}


void KatePluginIndexViewConfigPage::apply()
{
    emit configPageApplyRequest(this);
}


AboutParserWidget::AboutParserWidget(QWidget *parent/* = 0*/)
    : QWidget(parent)
{
    setupUi(this);
}


KatePluginIndexView::KatePluginIndexView(QObject *parent, const QList<QVariant>&)
    : KTextEditor::Plugin(parent)
{
}


KatePluginIndexView::~KatePluginIndexView()
{
}


QString KatePluginIndexView::name() const
{
    return i18n("Index View");
}

QString KatePluginIndexView::version() const
{
    return IndexViewVersion;
}

QIcon KatePluginIndexView::icon() const
{
    return IconCollection::getPluginIcon();
}


QObject *KatePluginIndexView::createView(KTextEditor::MainWindow *mainWindow)
{
    return new IndexView(this, mainWindow);
}


KTextEditor::ConfigPage *KatePluginIndexView::configPage(int number, QWidget *parent)
{
    if (number != 0) {
        return nullptr;
    }

    KatePluginIndexViewConfigPage *p = new KatePluginIndexViewConfigPage(this, parent);

    QSet<QString> parserClassAdded;
    bool loaded = false;

    for (auto view : m_views) {
        if (!loaded) {
            // No need to load/set for each view
            loaded = true;
            view->loadViewSettings();
            p->ui_filterBoxOnTop->setChecked(view->filterBoxPosition() == 2);
            p->ui_cozyClickExpand->setChecked(view->m_cozyClickExpand);
            p->ui_parseDelay->setValue(view->m_parseDelay);
        }

        for (auto parser : view->m_cache) {
            QString className = QLatin1String(parser->metaObject()->className());
            if (parserClassAdded.contains(className)) {
                continue;
            }
            parserClassAdded.insert(className);

            AboutParserWidget *w = new AboutParserWidget();
            w->ui_parserVersion->setText(parser->version());
            w->ui_parserAuthor->setText(parser->author());
            p->ui_parserTabs->addTab(w, className);
            if (view->parserOfCurrentView() == parser) {
                p->ui_parserTabs->setCurrentWidget(w);
            }
        }
    }

    p->ui_currentParserHint->setHidden(true);

    // It is unfortunately quite cumbersome to display this little extra hint in a sane way
    if (p->ui_parserTabs->count() > 1) {
        int marked = 0;
        p->ui_currentParserHint->setHidden(false);
        for (int i = 0; i < p->ui_parserTabs->count(); ++i) {
            for (auto view : m_views) {
                QString className = QLatin1String(view->parserOfCurrentView()->metaObject()->className());
                if (p->ui_parserTabs->tabText(i) == className) {
                    className.append(QStringLiteral(" *"));
                    p->ui_parserTabs->setTabText(i, className);
                    ++marked;
                    break;
                }
            }
        }

        if (marked > 1) {
            p->ui_currentParserHint->setText(i18n("The current parser in use are market with an asterisk *"));
        }
    }

    connect(p, &KatePluginIndexViewConfigPage::configPageApplyRequest, this, &KatePluginIndexView::applyConfig);

    return static_cast<KTextEditor::ConfigPage*>(p);
}


void KatePluginIndexView::applyConfig(KatePluginIndexViewConfigPage *p)
{
    bool saved = false;
    for (auto view : m_views) {
        int pos = p->ui_filterBoxOnTop->isChecked() ? 2 : 3;
        view->updateFilterBoxPosition(pos);

        view->m_cozyClickExpand = p->ui_cozyClickExpand->isChecked();
        view->m_parseDelay = p->ui_parseDelay->value();

        if (!saved) {
            // No need to save for each view
            saved = true;
            view->saveViewSettings();
        }
    }
}


K_PLUGIN_FACTORY_WITH_JSON(KatePluginIndexViewFactory, "kateindexviewplugin.json", registerPlugin<KatePluginIndexView>();)
#include "kate_plugin_index_view.moc"

// kate: space-indent on; indent-width 4; replace-tabs on;
