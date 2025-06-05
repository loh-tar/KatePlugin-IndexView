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


#ifndef KATE_PLUGIN_INDEX_VIEW_H
#define KATE_PLUGIN_INDEX_VIEW_H

#include <QSet>

#include <KLocalizedString>
#include <KTextEditor/ConfigPage>
#include <KTextEditor/Plugin>

#include "ui_kate_plugin_index_view_config_page.h"
#include "ui_kate_plugin_index_view_config_page_about_parser.h"

class KatePluginIndexView;

/**
 * Plugin's config page
 */
class KatePluginIndexViewConfigPage : public KTextEditor::ConfigPage
                                    , private Ui::IndexViewConfigWidget
{
    Q_OBJECT

    friend class KatePluginIndexView;

public:
    explicit KatePluginIndexViewConfigPage(KatePluginIndexView *plugin, QWidget *parentWidget = nullptr);
    ~KatePluginIndexViewConfigPage() override;

    /**
     * Reimplemented from KTextEditor::ConfigPage
     */
    QString name() const override;
    QString fullName() const override;
    QIcon icon() const override;

    void apply() override;
    void reset() override { ; }
    void defaults() override { ; }

Q_SIGNALS:
    /**
     * Ask the plugin to set initial values
     */
    void configPageApplyRequest(KatePluginIndexViewConfigPage*);

    /**
     * Ask the plugin to apply changes
     */
    void configPageInitRequest(KatePluginIndexViewConfigPage*);

private:
    KatePluginIndexView        *m_plugin;

};

class AboutParserWidget : public QWidget, public Ui::AboutParserWidget
{
public:
    AboutParserWidget(QWidget *parent = 0);
};

class IndexView;

class KatePluginIndexView : public KTextEditor::Plugin
{
    Q_OBJECT

    friend class KatePluginIndexViewConfigPage;
    friend class IndexView;

public:
    explicit KatePluginIndexView(QObject *parent = nullptr, const QList<QVariant>& = QList<QVariant>());
    ~KatePluginIndexView() override;

    QObject *createView(KTextEditor::MainWindow *mainWindow) override;

    int configPages() const override { return 1; }
    KTextEditor::ConfigPage *configPage(int number = 0, QWidget *parent = nullptr) override;

    QString name() const;
    QString version() const;
    QIcon icon() const;

public Q_SLOTS:
    void applyConfig(KatePluginIndexViewConfigPage *p);

private:
    QSet<IndexView *> m_views;

};

#endif

// kate: space-indent on; indent-width 4; replace-tabs on;
