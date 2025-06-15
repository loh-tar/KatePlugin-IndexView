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


CppParser::CppParser(QObject *view, KTextEditor::Document *doc)
    : ProgramParser(view, doc)
{
    using namespace IconCollection;
    // AccessSpecNode need special treatment, we store the action
    m_showAccessSpec = registerViewOption(AccessSpecNode, AccessSpecIcon, QStringLiteral("AccessSpec"), i18n("Show Access Specifiers"));
    registerViewOption(MacroNode, MacroIcon, QStringLiteral("Macros"), i18n("Show Macros"));
    registerViewOption(TypedefNode, TypedefIcon, QStringLiteral("Typedefs"), i18n("Show Typedefs"));
//     registerViewOption(VariableNode, VariableIcon, QStringLiteral("Variable"), i18n("Show Variables"));
    registerViewOption(EnumNode, EnumIcon, QStringLiteral("Enum"), i18n("Show Enums"));
    registerViewOption(FunctionDefNode, FuncDefIcon, QStringLiteral("Functions"), i18n("Show Function Def"));
//     m_showParameters  = registerViewOptionModifier(FunctionDefNode, QStringLiteral("ParameterF"), i18n("Show FParameter"));
    registerViewOption(FunctionDecNode, FuncDecIcon, QStringLiteral("Functions"), i18n("Show Function Dec"));
//     m_showParameters  = registerViewOptionModifier(FunctionDecNode, QStringLiteral("ParameterC"), i18n("Show CParameter"));

    setNodeTypeIcon(NamespaceNode, NamespaceIcon);
    setNodeTypeIcon(StructNode, ClassIcon);

    m_nonBlockElements << MacroNode << FunctionDecNode;

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
    // AccessSpecNode need special treatment
    nodeTypeIsWanted(AccessSpecNode);
    m_showAccessSpec->setEnabled(showAsTree());

    // https://en.cppreference.com/w/cpp/language/attributes
    static const QLatin1StringView rxAttribute(R"((\[\[.*\]\])?)");
    // https://en.cppreference.com/w/cpp/language/function
    // https://en.cppreference.com/w/cpp/language/declarations
    static const QLatin1StringView rxFuncDeclarator(R"((\W?[\s\w<\(\),:]*[&>\s\*]+)?)");

    // https://en.cppreference.com/w/cpp/language/classes
    static const QLatin1StringView rx1(R"(^(class|struct|union)%1[^:{]*\b(\w+)[:{])");
    static const QRegularExpression rxStruct(rx1.arg(rxAttribute));

    // https://en.cppreference.com/w/cpp/language/enum
    static const QLatin1StringView rx2(R"(\benum\W?(class|struct)?%1\W?(\w+)(:\w+)?\{)");
    static const QRegularExpression rxEnum(rx2.arg(rxFuncDeclarator));

    // FIXME Namespace part does not match Mul::ti::ple case
    static const QLatin1StringView rx3(R"(^%1(\w+::)*(\~?\w+)\(.*\)(.*)?\{)");
    static const QRegularExpression rxFuncDef(rx3.arg(rxFuncDeclarator), QRegularExpression::InvertedGreedinessOption);

    static const QLatin1StringView rx4(R"(^%1(\~?\w+)\(.*\)(.*)?;)");
    static const QRegularExpression rxFuncDec(rx4.arg(rxFuncDeclarator), QRegularExpression::InvertedGreedinessOption);

    // https://en.cppreference.com/w/c/language/typedef
    static const QRegularExpression rxTypedef(QStringLiteral(R"(\btypedef\s(\w+)\b(.+);)"));

    static const QRegularExpression rxNamespace(QStringLiteral(R"(^namespace (\w+))"));
    // https://en.cppreference.com/w/cpp/language/access.html
    static const QRegularExpression rxAccessSpec(QStringLiteral(R"(^((private|protected|public)( (\w+))?):)"));

    static const QRegularExpression typeDefStruct(QStringLiteral(R"(\btypedef struct( \w+)?\{)"));
    static const QRegularExpression typeDefStructA(QStringLiteral(R"(\btypedef struct (\w+) (\w+))"));

    QRegularExpressionMatch rxMatch;

    while (nextInstruction()) {

        static const QRegularExpression firstWordRx(QStringLiteral(R"(^[\W]*(\w+)\b)"));
        const QString firstWord = firstWordRx.match(m_line).captured(1);

        if (m_keywordsToIgnore.contains(firstWord)) {
            // Do nothing

        } else if (m_line.contains(rxNamespace, &rxMatch)) {
            addScopeNode(NamespaceNode, rxMatch.captured(1), m_lineNumber);

        } else if (m_line.contains(rxAccessSpec, &rxMatch)) {
            // qDebug() << rxMatch.captured(1) << rxMatch.captured(2) << rxMatch.captured(3) << rxMatch.captured(4);
            addAccessSpecNode(rxMatch.captured(1));

        } else if (m_line.contains(rxStruct, &rxMatch)) {
            addScopeNode(StructNode, rxMatch.captured(3), m_lineNumber);
            if (rxMatch.captured(1) == QStringLiteral("struct")) {
                addAccessSpecNode(QStringLiteral("public"));
            } else if (rxMatch.captured(1) == QStringLiteral("class")) {
                addAccessSpecNode(QStringLiteral("private"));
            } else /*if (rxMatch.captured(1) == QStringLiteral("union"))*/ {
                addAccessSpecNode(QStringLiteral("public"));
            }

        } else if (m_line.contains(rxEnum, &rxMatch)) {
            addNode(EnumNode, rxMatch.captured(3), m_lineNumber);

        } else if (m_line.contains(rxFuncDef, &rxMatch)) {
            addFuncDefNode(rxMatch.captured(1), rxMatch.captured(2), rxMatch.captured(3));

            // https://en.cppreference.com/w/cpp/language/function
            // "Function declarations may appear in any scope"...but
            // FIXME I didn't find a way to distinguish e.g. from creation of a new object
            // like: SomeClass foo(bar, baz);
            // so I add these parentNodeType check to have functions in header files but no stupid
            // stuff elsewhere. Edit: Also below namespace
        } else if ((parentNodeType() == AccessSpecNode || parentNodeType() == StructNode || parentNodeType() == NamespaceNode) && m_line.contains(rxFuncDec, &rxMatch)) {
        // } else if ((parentNodeType() == StructNode || parentNodeType() == NamespaceNode) && m_line.contains(rxFuncDec, &rxMatch)) {
        // } else if ((parentNodeType() == StructNode) && m_line.contains(rxFuncDec)) {
            addNode(FunctionDecNode, rxMatch.captured(2), m_lineNumber);

        } else if (m_line.contains(typeDefStruct)) {
                // We must fast forward to grep from the end of this instruction
                int lineNumber = m_lineNumber;
                while ((checkForBlocks() != 0) || !m_line.endsWith(QLatin1Char(';'))) {
                    if (!appendNextLine()) {
                        break;
                    }
                }
                m_lineNumber = Parser::lineNumber();
                stripLine();
                endOfBlock();
                m_line = m_line.section(QLatin1Char('}'), -1, -1);
                m_line.chop(1);
                addNode(TypedefNode, m_line, lineNumber);
                lastNode()->setData(0, NodeData::EndLine, m_lineNumber);

        } else if (m_line.contains(typeDefStructA, &rxMatch)) {
            // FIXME Urgs... Any idea to get rid of this extra special handling?
            // typedef struct tnode tnode; // tnode in ordinary name space is an alias to tnode in tag name space
            addNode(TypedefNode, rxMatch.captured(2), m_lineNumber);

        } else if (m_line.contains(rxTypedef, &rxMatch)) {
            static const QRegularExpression rxAlias(QStringLiteral(R"(([a-zA-Z_][\w]*))"));
            // Assume such case from cppreference.com
            //   typedef char char_t, *char_p, (*fp)(void);
            for (const QRegularExpressionMatch &match : rxAlias.globalMatch(rxMatch.captured(2))) {
                // FIXME I'm too stupid to exclude "void" already with the rxAlias
                if (match.captured(1) == QStringLiteral("void")) continue;
                addNode(TypedefNode, match.captured(1), m_lineNumber);
            }

        // } else {
        //     qDebug() << "NOPE" << m_lineNumber << m_line;
        }
    }
}


void CppParser::addAccessSpecNode(const QString &accessSpec)
{
    // AccessSpecNode need special treatment
    if (!m_showAccessSpec->isChecked() || !m_showAccessSpec->isEnabled()) {
        return;
    }

    QTreeWidgetItem *node = lastNode();
    while (node) {
        if (node->type() == StructNode) {
            break;
        }
        node = node->parent();
    }

    if (!node) {
        // Bad syntax in document
        return;
    }

    addScopeNode(node, AccessSpecNode, accessSpec);
}


void CppParser::addFuncDefNode(const QString &returnType, const QString &_nameSpace, const QString &funcName)
{
    const QString nameSpace = _nameSpace.section(QStringLiteral("::"), -9, -2); // Remove trailing ::

    // Don't add lambda functions but ctor and dtor
    if (returnType.isEmpty() && nameSpace.isEmpty()) {
        return;
    }

    addNodeToScope(nameSpace, NamespaceNode, FunctionDefNode, funcName, m_lineNumber);
}


bool CppParser::lineIsGood()
{
    if (ProgramParser::lineIsGood()) {
        return true;
    }

    // Despite the documentation to ProgramParser::lineIsGood(), I add these
    // quirk to solve some false detection of Q_FooMacro as function which has
    // occour when edit e.g. our index_view.h due to line concatenating
    static const QRegularExpression rx(QStringLiteral(R"(^Q_\w+)"));
    if (m_line.contains(rx)) {
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

    static const QRegularExpression rx(QStringLiteral(R"([^\\]\\$)"));
    while (m_line.contains(rx)) {
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
    static const QRegularExpression rxMarcro = QRegularExpression(QStringLiteral(R"(^#define (\w+))"));
    QRegularExpressionMatch rxMatch;
    if (m_line.contains(rxMarcro, &rxMatch)) {
        addNode(MacroNode, rxMatch.captured(1), m_lineNumber);
    }

    removeTrailingSharpComment();
    removeTrailingDoubleSlashComment();
}

// kate: space-indent on; indent-width 4; replace-tabs on;
