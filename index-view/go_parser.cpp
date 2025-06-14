/*   This file is part of KatePlugin-IndexView
 *
 *   GoParser Class
 *   Copyright (C) 2025 loh.tar@googlemail.com
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

#include "go_parser.h"


GoParser::GoParser(QObject *view, KTextEditor::Document *doc)
    : ProgramParser(view, doc)
{
    using namespace IconCollection;
    registerViewOption(StructNode, ClassIcon, QStringLiteral("Structs"), i18n("Show Structs"));
    registerViewOption(TypeNode, TypedefIcon, QStringLiteral("Types"), i18n("Show Types"));
    registerViewOption(InterfaceNode, InterfaceIcon, QStringLiteral("Interfaces"), i18n("Show Interfaces"));
    registerViewOption(FuncNode, FuncDefIcon, QStringLiteral("Functions"), i18n("Show Functions"));
//     registerViewOption(VariableNode, VariableIcon, QStringLiteral("Variable"), i18n("Show Variables"));
}


GoParser::~GoParser()
{
}


void GoParser::parseDocument()
{
    static const QRegularExpression rxStruct(QStringLiteral(R"(^type (\w+) struct)"));
    static const QRegularExpression rxInterface(QStringLiteral(R"(^type (\w+) interface)"));
    static const QRegularExpression rxType(QStringLiteral(R"(^type (\w+) (\w+))"));
    static const QRegularExpression rxFunc(QStringLiteral(R"(^func (\w+))"));
    static const QRegularExpression rxInterfaceDef(QStringLiteral(R"(^func\(\w+[\s\*](\w+)\)(\w+)\()"));

    QRegularExpressionMatch rxMatch;

    p_types.clear();

    while (nextInstruction()) {

        if (m_line.contains(rxStruct, &rxMatch)) {
            // qDebug() << rxMatch.captured(1) << rxMatch.captured(2) << rxMatch.captured(3);
            addNode(StructNode, rxMatch.captured(1), m_lineNumber);
            p_types.insert(rxMatch.captured(1), lastNode());

        } else if (m_line.contains(rxInterface, &rxMatch)) {
            addNode(InterfaceNode, rxMatch.captured(1), m_lineNumber);

        } else if (m_line.contains(rxType, &rxMatch)) {
            addNode(TypeNode, rxMatch.captured(1), m_lineNumber);
            p_types.insert(rxMatch.captured(1), lastNode());

        } else if (m_line.contains(rxFunc, &rxMatch)) {
            addNode(FuncNode, rxMatch.captured(1), m_lineNumber);

        } else if (m_line.contains(rxInterfaceDef, &rxMatch)) {
            addFuncToType(rxMatch.captured(1), rxMatch.captured(2));

        // } else {
            // qDebug() << "NOPE" << m_lineNumber << m_line;
        }
    }
}


void GoParser::addFuncToType(const QString &typeName, const QString &funcName)
{
    auto structNode = p_types.value(typeName, nullptr);
    if (!structNode) {
        // qDebug() << "FATAL: No p_types.value" << typeName;
        return;
    }

    auto *node = new QTreeWidgetItem(structNode, FuncNode);
    if (showAsTree()) {
        setNodeProperties(node, FuncNode, funcName, m_lineNumber);
    } else {
        setNodeProperties(node, FuncNode, structNode->text(0) + QStringLiteral(".") + funcName, m_lineNumber);
    }
}


bool GoParser::lineIsGood()
{
    // This looks surprising, not sure ATM what we should do, but it works not bad
    return true;
}


void GoParser::removeStrings()
{
    removeDoubleQuotedStrings();
    // FIXME Go use also back quotes, as in `foo`. Add a function to ProgramParser if needed
}


void GoParser::removeComment()
{
    removeMultiLineSlashStarComment();
    removeTrailingDoubleSlashComment();
}

// kate: space-indent on; indent-width 4; replace-tabs on;
