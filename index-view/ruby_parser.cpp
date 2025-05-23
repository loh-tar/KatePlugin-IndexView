/*   This file is part of KatePlugin-IndexView
 *
 *   RubyParser Class
 *   Copyright (C) 2018 loh.tar@googlemail.com
 *
 *   Inspired by ruby_parser.cpp, part of Kate's SymbolViewer
 *   Copyright (C) 2007 Massimo Callegari <massimocallegari@yahoo.it>
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


#include <KLocalizedString>

#include "icon_collection.h"
#include "index_view.h"

#include "ruby_parser.h"


RubyParser::RubyParser(QObject *view, const QString &docType, KTextEditor::Document *doc)
    : ProgramParser(view, docType, doc)
{
    using namespace IconCollection;
    registerViewOption(ClassNode, ClassIcon, QStringLiteral("Classes"), i18n("Show Classes"));
    registerViewOption(MixinNode, Green1Icon, QStringLiteral("Mixin"), i18n("Show Mixin"));
    registerViewOption(ModuleNode, Green2Icon, QStringLiteral("Modules"), i18n("Show Modules"));
    registerViewOption(MethodNode, MethodIcon, QStringLiteral("Methods"), i18n("Show Methods"));
    m_showParameters = registerViewOptionModifier(MethodNode, QStringLiteral("Parameters"), i18n("Show Parameter"));

    addViewOptionDependency(MixinNode, ClassNode);
    m_nonBlockElements << MixinNode;
    m_blockElements << QStringLiteral("if") << QStringLiteral("case") << QStringLiteral("for");// << QStringLiteral("") << QStringLiteral("");
}


RubyParser::~RubyParser()
{
}


void RubyParser::parseDocument()
{
    static const QRegularExpression rxEndBlock(QStringLiteral("^end(\\s)*$"));
    // TODO Use rx/match too

    while (nextInstruction()) {
        if (m_line.startsWith(QStringLiteral("class "))) {
            addNode(ClassNode, m_line.mid(6), m_lineNumber);
            beginOfBlock();

        } else if (m_line.startsWith(QStringLiteral("module "))) {
            addNode(ModuleNode, m_line.mid(7), m_lineNumber);
            beginOfBlock();

        } else if (m_line.startsWith(QStringLiteral("def "))) {
            m_line = m_line.mid(4);
            if (!m_showParameters->isChecked()) {
                m_line = m_line.section(QLatin1Char('('), 0, 0);
            }
            addNode(MethodNode, m_line, m_lineNumber);
            beginOfBlock();

        } else if (m_line.startsWith(QStringLiteral("include "))) {
            addNode(MixinNode, m_line.mid(8), m_lineNumber);

        } else if (m_line.startsWith(QStringLiteral("prepend "))) {
            addNode(MixinNode, m_line.mid(8), m_lineNumber);

        } else if (m_line.contains(rxEndBlock)) {
            endOfBlock();

        } else {
            m_line = m_line.section(QLatin1Char(' '), 0, 0);
            if (m_blockElements.contains(m_line)) {
                beginOfBlock();
            }
        }
    }
}

// kate: space-indent on; indent-width 4; replace-tabs on;
