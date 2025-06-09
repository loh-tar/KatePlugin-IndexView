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


#ifndef INDEXVIEW_GOPARSER_CLASS_H
#define INDEXVIEW_GOPARSER_CLASS_H

#include "program_parser.h"

/**
 * The \p GoParser provide support for Go files.
 *
 * @author loh.tar
 */
class GoParser : public ProgramParser
{
    Q_OBJECT

public:
    GoParser(QObject *view, KTextEditor::Document *doc);
   ~GoParser();

protected:
   enum NodeType {
        StructNode = FirstNodeType,
        TypeNode,
        InterfaceNode,
        FuncNode
        // VariableNode,
    };

    QString version() override { return QStringLiteral("0.6, Jul 2025"); } ;
    QString author() override { return QStringLiteral("2025 loh.tar"); } ;

    void parseDocument() override;
    void addFuncToType(const QString &typeName, const QString &funcName);

    bool lineIsGood() override;
    void removeStrings() override;
    void removeComment() override;

    // QAction                                 *m_showParameters; // FIXME if you really need need it
    QHash<QString, QTreeWidgetItem*>            p_types; // For easy adding of functions to types
};

#endif

// kate: space-indent on; indent-width 4; replace-tabs on;
