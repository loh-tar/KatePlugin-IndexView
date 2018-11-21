/*   This file is part of KatePlugin-IndexView
 *
 *   CppParser Class
 *   Copyright (C) 2018 loh.tar@googlemail.com
 *
 *   Inspired by cpp_parser.cpp, part of Kate's SymbolViewer
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


#include <KLocalizedString>

#include "icon_collection.h"
#include "index_view.h"

#include "cpp_parser.h"


CppParser::CppParser(IndexView *view)
    : ProgramParser(view)
{
    using namespace IconCollection;
    registerViewOption(NamespaceNode, NamespaceIcon, QStringLiteral("Namespace"), i18n("Show Namespace"));
    registerViewOption(MacroNode, SmallGreenIcon, QStringLiteral("Macros"), i18n("Show Macros"));
    registerViewOption(TypedefNode, SmallRedIcon, QStringLiteral("Typedefs"), i18n("Show Typedefs"));
//     registerViewOption(VariableNode, VariableIcon, QStringLiteral("Variable"), i18n("Show Variables"));
    registerViewOption(StructNode, ClassIcon, QStringLiteral("Structs"), i18n("Show Structs"));
    registerViewOption(FunctionDefNode, FunctionIcon, QStringLiteral("Functions"), i18n("Show Functions"));
//     m_showParameters  = registerViewOptionModifier(FunctionDefNode, QStringLiteral("ParameterF"), i18n("Show FParameter"));
    registerViewOption(FunctionDecNode, FunctionIcon, QStringLiteral("Functions"), i18n("Show Functions"));
//     m_showParameters  = registerViewOptionModifier(FunctionDecNode, QStringLiteral("ParameterC"), i18n("Show CParameter"));

    m_nonBlockElements << MacroNode << FunctionDecNode;

    // https://en.cppreference.com/w/cpp/language/attributes
    QString rxAttribute = QStringLiteral("(\\[\\[.*\\]\\])?");
    // https://en.cppreference.com/w/cpp/language/function
    // https://en.cppreference.com/w/cpp/language/declarations
    QString rxFuncDeclarator = QStringLiteral("(\\W?[\\s\\w<\\(\\),:]*[>\\s\\*]+)?");

    // https://en.cppreference.com/w/cpp/language/classes
    QString rx = QStringLiteral("^(class|struct|union)%1[^:{]*\\b(\\w+)[:{]");
    m_rxStruct = QRegExp(rx.arg(rxAttribute));

    // https://en.cppreference.com/w/cpp/language/enum
    rx = QStringLiteral("\\benum\\W?(class|struct)?%1\\W?(\\w+)(:\\w+)?\\{");
    m_rxEnum = QRegExp(rx.arg(rxFuncDeclarator));

    rx = QStringLiteral("^%1(\\w+::)*(\\~?\\w+)\\(.*\\)(.*)?\\{");
    m_rxFuncDef = QRegExp(rx.arg(rxFuncDeclarator));
    m_rxFuncDef.setMinimal(true);

    rx = QStringLiteral("^%1(\\~?\\w+)\\(.*\\)(.*)?;");
    m_rxFuncDec = QRegExp(rx.arg(rxFuncDeclarator));
    m_rxFuncDec.setMinimal(true);

    // https://en.cppreference.com/w/c/language/typedef
    m_rxTypedef = QRegExp(QStringLiteral("\\btypedef\\b(.*)\\b(\\S+);"));

    m_rxMarcro = QRegExp(QStringLiteral("^#define (\\w+)"));

    m_rxNamespace= QRegExp(QStringLiteral("^namespace (\\w+)"));

    m_keywordsToIgnore // To avaoid false detection, e.g. as function
        << QStringLiteral("Q_FOREACH")
        << QStringLiteral("catch")
        << QStringLiteral("else")
        << QStringLiteral("for")
        << QStringLiteral("foreach")
        << QStringLiteral("if")
        << QStringLiteral("switch")
        << QStringLiteral("until")
    //     << QStringLiteral("")
        << QStringLiteral("while");

}


CppParser::~CppParser()
{
}


void CppParser::parseDocument()
{
    while (nextInstruction()) {

        const QString firstWord = m_line.section(QRegExp(QStringLiteral("\\b")), 1, 1);

        if (m_keywordsToIgnore.contains(firstWord)) {
            // Do nothing

        } else if (m_line.contains(m_rxNamespace)) {
            addNode(NamespaceNode, m_rxNamespace.cap(1), m_lineNumber);

        } else if (m_line.contains(m_rxStruct)) {
            addNode(StructNode, m_rxStruct.cap(3), m_lineNumber);

        } else if (m_line.contains(m_rxEnum)) {
            addNode(StructNode, m_rxEnum.cap(3), m_lineNumber);

        } else if (m_line.contains(m_rxFuncDef)) {
            addNode(FunctionDefNode, m_rxFuncDef.cap(3), m_lineNumber);

            // https://en.cppreference.com/w/cpp/language/function
            // "Function declarations may appear in any scope"...but
            // FIXME I didn't find a way to distinguish e.g. from creation of a new object
            // like: SomeClass foo(bar, baz);
            // so I add these parentNodeType check to have functions in header files but no stupid
            // stuff elsewhere. Edit: Also below namespace
        } else if ((parentNodeType() == StructNode || parentNodeType() == NamespaceNode) && m_line.contains(m_rxFuncDec)) {
    //  } else if ((parentNodeType() == StructNode) && m_line.contains(m_rxFuncDec)) {
            addNode(FunctionDecNode, m_rxFuncDec.cap(2), m_lineNumber);

        } else if (m_line.contains(QRegExp(QStringLiteral("\\btypedef struct( \\w+)?\\{")))) {
                // We must fast forward to grep from the end of this instruction
                int lineNumber = m_lineNumber;
                while ((checkForBlocks() != 0) || !m_line.endsWith(QLatin1Char(';'))) {
                    if (!appendNextLine()) {
                        break;
                    }
                    stripLine();
                }
                endOfBlock();
                m_line = m_line.section(QLatin1Char('}'), -1, -1);
                m_line.chop(1);
                addNode(TypedefNode, m_line, lineNumber);

        } else if (m_line.contains(m_rxTypedef)) {
            addNode(TypedefNode, m_rxTypedef.cap(2), m_lineNumber);

        }
    }
}


bool CppParser::lineIsGood()
{
    if (ProgramParser::lineIsGood()) {
        return true;
    }

    // Despite the documentation to ProgramParser::lineIsGood(), I add these
    // quirk to solve some false detection of Q_FooMacro as function which has
    // occour when edit e.g. our index_view.h due to line concatenating
    if (m_line.contains(QRegExp(QStringLiteral("^Q_\\w+")))) {
        return true;
    }

    return false;
}


bool CppParser::appendNextLine()
{
    // C++ has line continuation by backslash in a very early state
    // https://stackoverflow.com/a/7059563 (didn't find it on cppreference.com)
    if (!Parser::appendNextLine()) {
        return false;
    }

    while (m_line.contains(QRegExp(QStringLiteral("[^\\\\]\\\\$")))) {
        m_line.chop(1);
        if(!incrementLineNumber()) {
            break;
        }
        m_line.append(rawLine());
    }

    return true;
}


void CppParser::removeStrings()
{
    removeSingleQuotedStrings();
    removeDoubleQuotedStrings();
}


void CppParser::removeComment()
{
    removeMultiLineSlashStarComment();

    if (m_line.contains(m_rxMarcro)) {
        addNode(MacroNode, m_rxMarcro.cap(1), m_lineNumber);
    }

    removeTrailingSharpComment();
    removeTrailingDoubleSlashComment();
}

// kate: space-indent on; indent-width 4; replace-tabs on;
