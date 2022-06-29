/*   This file is part of KatePlugin-IndexView
 *
 *   PhpParser Class
 *   Copyright (C) 2018 loh.tar@googlemail.com
 *
 *   Inspired by php_parser.cpp, part of Kate's SymbolViewer
 *   Copyright (C) 2010 Emmanuel Bouthenot <kolter@openics.org>
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

#include "php_parser.h"


PhpParser::PhpParser(IndexView *view)
    : ProgramParser(view)
{
    using namespace IconCollection;
    registerViewOption(ClassNode, ClassIcon, QStringLiteral("Classes"), i18n("Show Classes"));
    registerViewOption(InterfaceNode, BlueIcon, QStringLiteral("Interfaces"), i18n("Show Interfaces"));
    registerViewOption(NamespaceNode, NamespaceIcon, QStringLiteral("Namespace"), i18n("Show Namespace"));
    registerViewOption(ConstantNode, ConstantIcon, QStringLiteral("Constants"), i18n("Show Constants"));
    registerViewOption(FunctionNode, FunctionIcon, QStringLiteral("Functions"), i18n("Show Functions"));
    registerViewOption(VariableNode, VariableIcon, QStringLiteral("Variables"), i18n("Show Variables"));

    m_nonBlockElements << ConstantNode << VariableNode;//<< NamespaceNode;

    m_rxDefine = QRegularExpression(QStringLiteral("(?:^|\\W)define\\s*\\(\\s*['\"]([^'\"]+)['\"]"), QRegularExpression::CaseInsensitiveOption);
    m_rxConst = QRegularExpression(QStringLiteral("^(?:\\w+ )?const ([\\w_][\\w\\d_]*)"), QRegularExpression::CaseInsensitiveOption);
    m_rxNamespace = QRegularExpression(QStringLiteral("^namespace ([\\w/]+)"), QRegularExpression::CaseInsensitiveOption);
    m_rxClass = QRegularExpression(QStringLiteral("\\bclass (\\w+)\\b.*\\{"), QRegularExpression::CaseInsensitiveOption);
    m_rxInterface = QRegularExpression(QStringLiteral("^interface (\\w+)\\b.*\\{"), QRegularExpression::CaseInsensitiveOption);
    m_rxFunction = QRegularExpression(QStringLiteral("\\bfunction (\\w+)\\("), QRegularExpression::CaseInsensitiveOption);

    // After reading these quoted post, I added variables in a very eager way
    //  > I highly recommend to use an editor that can list all variable names in a separate window.
    //  > The reason are typing errors in variable names.
    //  > https://php.net/manual/en/language.variables.basics.php#120617
    m_rxVariable = QRegularExpression(QStringLiteral("\\$(\\w+)"), QRegularExpression::CaseInsensitiveOption);

    initHereDoc(QStringLiteral("<<<"), QStringLiteral("'"));
}


PhpParser::~PhpParser()
{
}


void PhpParser::parseDocument()
{
    QRegularExpressionMatch rxMatch;

    while (nextInstruction()) {

        if (m_line.contains(m_rxNamespace, &rxMatch)) {
            addNode(NamespaceNode, rxMatch.captured(1), m_lineNumber);

        } else if (m_line.contains(QStringLiteral("define(,);"))) {
            m_niceLine.contains(m_rxDefine, &rxMatch);
            addNode(ConstantNode, rxMatch.captured(1), m_lineNumber);

        } else if (m_line.contains(m_rxConst, &rxMatch)) {
            addNode(ConstantNode, rxMatch.captured(1), m_lineNumber);

        } else if (m_line.contains(m_rxClass, &rxMatch)) {
            addNode(ClassNode, rxMatch.captured(1), m_lineNumber);

        } else if (m_line.contains(m_rxInterface, &rxMatch)) {
            addNode(InterfaceNode, rxMatch.captured(1), m_lineNumber);

        } else if (m_line.contains(m_rxVariable, &rxMatch)) {
            addNode(VariableNode, rxMatch.captured(1), m_lineNumber);

        } else if (m_line.contains(m_rxFunction, &rxMatch)) {
            addNode(FunctionNode, rxMatch.captured(1), m_lineNumber);

        }
    }
}


void PhpParser::removeStrings()
{
    removeDoubleQuotedStrings();
    removeSingleQuotedStrings();

    // Ignore everything until we had read php open tag
    if (m_line.contains(QStringLiteral("<?php"))) {
        m_line = m_line.section(QStringLiteral("<?php"), -1, -1);
        m_line = m_line.trimmed();
        m_funcAtWork.insert(Me_At_Work);
    }

    if (m_funcAtWork.contains(Me_At_Work)) {
        if (m_line.contains(QStringLiteral("?>"))) {
            m_line = m_line.section(QStringLiteral("?>"), 0, 0);
            m_funcAtWork.remove(Me_At_Work);
        }
    } else {
        m_line.clear();
    }

    removeHereDoc();
}


void PhpParser::removeComment()
{
    removeMultiLineSlashStarComment();
    removeTrailingDoubleSlashComment();
    removeTrailingSharpComment();
}

// kate: space-indent on; indent-width 4; replace-tabs on;
