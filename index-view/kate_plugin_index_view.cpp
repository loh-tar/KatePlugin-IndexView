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


#define IndexViewVersion QStringLiteral("0.8.3, Dez 2018")


KatePluginIndexViewConfigPage::KatePluginIndexViewConfigPage(KatePluginIndexView *plugin, QWidget *parentWidget /*= 0L*/)
    : KTextEditor::ConfigPage(parentWidget)
    , m_plugin(plugin)
{
    setupUi(this);
    ui_pluginVersion->setText(IndexViewVersion);
    ui_pluginAuthor->setText(QStringLiteral("2018 loh.tar \n\nBased on SymbolViewer \n2014 Kåre Särs \n2003 Massimo Callegari"));
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


QIcon KatePluginIndexView::icon() const
{
    return IconCollection::getIcon(IconCollection::GreenYellowIcon);
}


QObject *KatePluginIndexView::createView(KTextEditor::MainWindow *mainWindow)
{
    m_view = new IndexView(this, mainWindow);
    return m_view;
}


KTextEditor::ConfigPage *KatePluginIndexView::configPage(int number, QWidget *parent)
{
    if (number != 0) {
        return nullptr;
    }

    KatePluginIndexViewConfigPage *p = new KatePluginIndexViewConfigPage(this, parent);
    if (m_view) {
        m_view->loadViewSettings();

        p->ui_filterBoxOnTop->setChecked(m_view->filterBoxPosition() == 0);
        p->ui_cozyClickExpand->setChecked(m_view->m_cozyClickExpand);
        p->ui_parseDelay->setValue(m_view->m_parseDelay);

        if (m_view->m_parser) {
            p->ui_aboutParserBox->setTitle(i18n("About %1", QLatin1String(m_view->m_parser->metaObject()->className())));
            p->ui_parserVersion->setText(m_view->m_parser->version());
            p->ui_parserAuthor->setText(m_view->m_parser->author());
        }
    }

    connect(p, &KatePluginIndexViewConfigPage::configPageApplyRequest, this, &KatePluginIndexView::applyConfig);

    return static_cast<KTextEditor::ConfigPage*>(p);
}


void KatePluginIndexView::applyConfig(KatePluginIndexViewConfigPage *p)
{
    if (m_view) {
        int pos = p->ui_filterBoxOnTop->isChecked() ? 0 : 1;
        m_view->updateFilterBoxPosition(pos);

        m_view->m_cozyClickExpand = p->ui_cozyClickExpand->isChecked();
        m_view->m_parseDelay = p->ui_parseDelay->value();

        m_view->saveViewSettings();
    }
}


K_PLUGIN_FACTORY_WITH_JSON(KatePluginIndexViewFactory, "kateindexviewplugin.json", registerPlugin<KatePluginIndexView>();)
#include "kate_plugin_index_view.moc"

// kate: space-indent on; indent-width 4; replace-tabs on;
