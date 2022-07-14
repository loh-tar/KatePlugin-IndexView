/*   This file is part of KatePlugin-IndexView
 *
 *   ProgramParser Class
 *   Copyright (C) 2018 loh.tar@googlemail.com
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


#include <QDebug>

#include <KLocalizedString>

#include "index_view.h"

#include "program_parser.h"


ProgramParser::ProgramParser(IndexView *view)
    : Parser(view)
{
    // Because it's not a full tree with one single common root, let's rename it slightly
    p_viewTree->setText(i18n("Structure View"));
    p_viewTree->setObjectName(QStringLiteral("StructureView"));

    using namespace IconCollection;
    registerViewOption(FixmeTodoNode, FixmeTodoIcon, QStringLiteral("FIXME/TODO"), i18n("Show FIXME/TODO"));
    registerViewOption(BeginNode, BeginIcon, QStringLiteral("BEGIN"), i18n("Show BEGIN"));
    m_detachedNodeTypes << FixmeTodoNode << BeginNode;
}


ProgramParser::~ProgramParser()
{
}


void ProgramParser::parse()
{
    p_parentNode = nullptr;
    p_scopeRoots.clear();
    clearNesting();

    p_bracesDelta = 0;

    Parser::parse();
}


bool ProgramParser::nextInstruction()
{
    checkNesting();

    m_line.clear();
    // Set m_lineNumber always to the first non empty line read
    m_lineNumber = Parser::lineNumber() + 1;

    while (appendNextLine()) {

        stripLine();

        if (m_line.isEmpty()) {
            m_lineNumber = Parser::lineNumber() + 1;
            continue;
        }

        if (lineIsGood()) {
            break;
        }
    }

    checkForBlocks();

// qDebug() << "  " << m_lineNumber + 1 << m_line.left(35);

    return !m_line.isEmpty();
}


bool ProgramParser::lineIsGood()
{
    if (m_funcAtWork.contains(Line_Is_Good)) {
        m_funcAtWork.remove(Line_Is_Good);
        return true;
    }
    static const QRegularExpression rx1(QStringLiteral("[{};]$"));
    if (m_line.contains(rx1)) {
        return true;
    }

    static const QRegularExpression rx2(QStringLiteral("^[\\w\\s]+:$"));
    // Match labels like, "protected:" or "public Q_SLOTS:"
    if (m_line.contains(rx2)) {
        return true;
    }

    return false;
}


void ProgramParser::checkNesting()
{
//     qDebug() << "ProgramParser::checkNesting() >>>";
    for (int j = p_bracesDelta; j > 0; j--) {
        beginOfBlock();
    }
    for (int j = p_bracesDelta; j < 0; j++) {
        endOfBlock();
    }

    p_bracesDelta = 0;
//     qDebug() << "ProgramParser::checkNesting() <<<";
}


void ProgramParser::stripLine()
{
    m_line = m_line.simplified();
    m_niceLine = m_line;// FIXME It's not only nice when m_line will append
    removeStrings();

    const int sizeWithComment = m_line.size();
    removeComment();
    // Add fixme/todo nodes to the index
    if (sizeWithComment > m_line.size()) {
        static const QStringList tags = { QStringLiteral("FIXME|TODO"), QStringLiteral("BEGIN") };
        static const QList<int> nodes = { FixmeTodoNode, BeginNode };
        for (int i = 0; i < tags.size(); ++i) {
            if (addCommentTagNode(tags.at(i), nodes.at(i))) {
                break;
            }
        }
    }

    // Squash the line, remove all unneeded space
    static const QRegularExpression rx(QStringLiteral("(\\s)?(\\W)(\\s)?"));
    m_line.replace(rx, QStringLiteral("\\2"));
}


bool ProgramParser::addCommentTagNode(const QString &tag, const int nodeType)
{
    // FIXME ? No static here, but could be done when we again check what kind of tag it is
    const QRegularExpression regEx = QRegularExpression(QStringLiteral("(.*)\\b(%1)\\b(.*)?").arg(tag));
    const QRegularExpressionMatch rxMatch = regEx.match(rawLine());
    if (!rxMatch.hasMatch()) {
        return false;
    }

    if (!nodeTypeIsWanted(nodeType)) {
        return true;
    }

    // Support also notes where the token is at the end
    // FIXME This solutuon is not best. Better is the trick done in XmlTypeParser but that can we not do
    // here because we have (yet) no "clean" comment string. But we could collect one in our removeComment functions
    QString txt = rxMatch.captured(3).isEmpty() ? rxMatch.captured(1) : rxMatch.captured(3);
    // Remove possible comment char from both ends in a lazy way. So,
    // something too much could be gone but guess it's OK
    static const QRegularExpression rx1(QStringLiteral("^\\W*"));
    txt.remove(rx1);
    static const QRegularExpression rx2(QStringLiteral("\\W*$"));
    txt.remove(rx2);
    addNode(nodeType, rxMatch.captured(2).at(0) + QStringLiteral(": ") + txt, m_lineNumber);

    return true;
}


void ProgramParser::removeStrings()
{
    removeDoubleQuotedStrings();
}


void ProgramParser::removeDoubleQuotedStrings()
{
    // Thanks to https://stackoverflow.com/a/5696141
    static const QRegularExpression rx(QStringLiteral("\"[^\"\\\\]*(?:\\\\.[^\"\\\\]*)*\""));
    m_line.remove(rx);
    // Not sure if complete deletion is best or keeping a dummy
//     m_line.replace(QRegularExpression(QStringLiteral("\"[^\"\\\\]*(?:\\\\.[^\"\\\\]*)*\"")), QStringLiteral("X"));
}


void ProgramParser::removeSingleQuotedStrings()
{
    // Thanks to https://stackoverflow.com/a/5696141
    static const QRegularExpression rx(QStringLiteral("\'[^\'\\\\]*(?:\\\\.[^\'\\\\]*)*\'"));
    m_line.remove(rx);
}


void ProgramParser::removeMultiLineSlashStarComment()
{
    if (m_funcAtWork.contains(Me_At_Work)) {
        if (m_line.contains(QStringLiteral("*/"))) {
            m_line = m_line.section(QStringLiteral("*/"), -1, -1);
            m_funcAtWork.remove(Me_At_Work);
        } else {
            m_line.clear();
        }
    }

    removeInLineSlashStarComment();

    if (m_line.contains(QStringLiteral("/*"))) {
        m_line = m_line.section(QStringLiteral("/*"), 0, 0);
        m_funcAtWork.insert(Me_At_Work);
    }
}


void ProgramParser::removeInLineSlashStarComment()
{
    static QRegularExpression rx(QStringLiteral("/\\*.*\\*/"), QRegularExpression::InvertedGreedinessOption);
//     rx.setMinimal(true);

    m_line.remove(rx);
}


void ProgramParser::removeComment()
{
    removeTrailingSharpComment();
}


void ProgramParser::removeTrailingSharpComment()
{
    m_line = m_line.section(QLatin1Char('#'), 0, 0);
}


void ProgramParser::removeTrailingDoubleSlashComment()
{
    m_line = m_line.section(QStringLiteral("//"), 0, 0);
}


void ProgramParser::initHereDoc(const QString &hereDocOperator, const QString &quoteChars)
{
    p_rxHereDocOperator = QRegularExpression(hereDocOperator);

    // Build for each quote char an own RegExp
    QRegularExpression rx;
    for (int i = 0; i < quoteChars.size(); ++i) {
// qDebug() << "HereDocQuote:" << quoteChars.at(i);
        rx = QRegularExpression(QStringLiteral("%1\\s?%2(.+)%2").arg(hereDocOperator).arg(quoteChars.at(i)), QRegularExpression::InvertedGreedinessOption);
//         rx.setMinimal(true);
        p_hereDocRxList << rx;
    }
    // Add at last a special RegEx for unquoted tokens
    // FIXME To avoid a couple of false catch, this regex may be more limited as useful
    // Problem was ";" in Perl and left shift a number "<< 20"
    rx = QRegularExpression(QStringLiteral("%1\\s?([A-Za-z][^\\s;]*)").arg(hereDocOperator));
    p_hereDocRxList << rx;
}


void ProgramParser::removeHereDoc()
{
    // Not all heredoc syntax may could be catch with this approach
    // https://en.wikipedia.org/wiki/Here_document
    static QString token;

    if (m_funcAtWork.contains(Me_At_Work)) {
        if (m_line.startsWith(token)) {
            m_funcAtWork.remove(Me_At_Work);
// qDebug() << "DONE:" << token;
        }
        m_funcAtWork.insert(Line_Is_Good);
        m_line.clear();

    // Check in two steps to keep the regexp simple
    } else if (m_line.contains(p_rxHereDocOperator)) {
    // Now we are alomost sure to have a hit, we need to check against m_niceLine
    // Due to Perl's possibility to stack multible here docs we must test all
    // in p_hereDocRxList and choose the right most hit.
        int lastIndexIn = -1;
        int indexIn = -1;
        for (QRegularExpression rx : qAsConst(p_hereDocRxList)) {
            indexIn = m_niceLine.indexOf(rx);
            if (indexIn < 0) {
                continue;
            }
            if (indexIn > lastIndexIn) {
                lastIndexIn = indexIn;
                QRegularExpressionMatch rxMatch = rx.match(m_niceLine);
                token = rxMatch.captured(1).trimmed();
// qDebug() << "TOKEN:" << token << rxMatch.lastCapturedIndex();
            }
        }
        if (lastIndexIn != -1) {
            m_funcAtWork.insert(Me_At_Work);
            m_funcAtWork.insert(Line_Is_Good);
        }
    }
}


int ProgramParser::checkForBlocks()
{
    int openBraces   = m_line.count(QLatin1Char('{'));
    int closeBraces  = m_line.count(QLatin1Char('}'));

    p_bracesDelta = openBraces - closeBraces;

    if (!p_bracesDelta && closeBraces) {
        // Catch lines like: "} else {"
        if (m_line.startsWith(QLatin1Char('}'))) {
            // Close bock in advance
            endOfBlock();
            --closeBraces;
            p_bracesDelta = openBraces - closeBraces;
        }
    }

    return p_bracesDelta;
}


void ProgramParser::beginOfBlock()
{
    QTreeWidgetItem *node = lastNode();

    if (!p_nestingStack.isEmpty()) {
        if (p_nestingStack.top() == node) {
            // Ignore this block
            if (p_nestingFoo < 0) {
                p_nestingFoo = p_nestingStack.size() + 1;
            }
        }
    }

    if (node && node->data(0, NodeData::EndLine).toInt() != -1) {
        // Ignore this block
        if (p_nestingFoo < 0) {
            p_nestingFoo = p_nestingStack.size() + 1;
        }
    }

    p_parentNode = node;
    p_nestingStack.push(node);
}


void ProgramParser::endOfBlock()
{
    if (p_nestingFoo >= p_nestingStack.size()) {
        p_nestingFoo = -1;
    }

    if (p_nestingStack.isEmpty()) {
        p_parentNode = nullptr;
        return;
    }

    p_parentNode = p_nestingStack.pop();
    if (!p_parentNode) {
        return;
    }

    if (!p_nestingStack.isEmpty() && p_nestingStack.top() == p_parentNode) {
        return;
    } else {
        p_parentNode->setData(0, NodeData::EndLine, lineNumber());
    }

    p_parentNode = p_parentNode->parent();
}


void ProgramParser::clearNesting()
{
    p_nestingFoo = -1;
    p_nestingStack.clear();
}


void ProgramParser::addNode(const int nodeType, const QString &text, const int lineNumber, const int columnNumber/* = 0*/)
{
    QTreeWidgetItem *node = nullptr;

    if (m_detachedNodeTypes.contains(nodeType)) {
        node = new QTreeWidgetItem(rootNode(nodeType), nodeType);
        setNodeProperties(node, nodeType, text, lineNumber, columnNumber);
        return;
    }

    if (!nodeTypeIsWanted(nodeType)) {
        return;
    }
    if (p_nestingFoo > -1) {
        return;
    }

    if (!p_parentNode || p_nestingStack.isEmpty()) {
        p_parentNode = rootNode(nodeType);
    }
    if (m_nonBlockElements.contains(p_parentNode->type()) && p_parentNode->type() != nodeType) {
        p_parentNode = p_rootNodes.value(nodeType);
    }
    node = new QTreeWidgetItem(p_parentNode, nodeType);

    setNodeProperties(node, nodeType, text, lineNumber, columnNumber);

    if (m_nonBlockElements.contains(nodeType)) {
        node->setData(0, NodeData::EndLine, lineNumber);
    }
}


void ProgramParser::addNodeToScope(const QString &scope, const int scopeType, const int nodeType, const QString &text, const int lineNumber, const int columnNumber/* = 0*/)
{
    if (!nodeTypeIsWanted(nodeType)) {
        return;
    }

    if (scope.isEmpty()) {
        addNode(nodeType, text, lineNumber, columnNumber);
        return;
    }

    QTreeWidgetItem *scopeRoot = p_scopeRoots.value(scope, nullptr);

    if (!scopeRoot) {
        scopeRoot = new QTreeWidgetItem(p_indexTree, nodeType);
        setNodeProperties(scopeRoot, scopeType, scope, -1);
        p_scopeRoots.insert(scope, scopeRoot);
    }

    QTreeWidgetItem *node = new QTreeWidgetItem(scopeRoot, nodeType);
    setNodeProperties(node, nodeType, text, lineNumber, columnNumber);
}


void ProgramParser::addScopeNode(const int nodeType, const QString &text, const int lineNumber, const int columnNumber/* = 0*/)
{
    // Disabled. Guess it is unlikely we have an option for that.
    // Why should we not want such important node see?
    // if (!nodeTypeIsWanted(nodeType)) {
    //     return;
    // }

    QTreeWidgetItem *node = nullptr;

    if (!p_parentNode || p_nestingStack.isEmpty()) {
        node = p_scopeRoots.value(text, nullptr);

        if (node) {
            // Hm, really nothing else todo?
            return;
        }

        node = new QTreeWidgetItem(p_indexTree, nodeType);
        p_scopeRoots.insert(text, node);
        p_parentNode = node; // That's OK?

    } else {
        node = new QTreeWidgetItem(p_parentNode, nodeType);
    }

    setNodeProperties(node, nodeType, text, lineNumber, columnNumber);
}

// kate: space-indent on; indent-width 4; replace-tabs on;
