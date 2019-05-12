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
    registerViewOption(FixmeTodoNode, SmallYellowIcon, QStringLiteral("FIXME/TODO"), i18n("Show FIXME/TODO"));
    registerViewOption(BeginNode, BlueIcon, QStringLiteral("BEGIN"), i18n("Show BEGIN"));
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

    if (m_line.contains(QRegExp(QStringLiteral("[{};]$")))) {
        return true;
    }

    // Match lables like, "protected:" or "public Q_SLOTS:"
    if (m_line.contains(QRegExp(QStringLiteral("^[\\w\\s]+:$")))) {
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
    m_line.replace(QRegExp(QStringLiteral("(\\s)?(\\W)(\\s)?")), QStringLiteral("\\2"));
}


bool ProgramParser::addCommentTagNode(const QString &tag, const int nodeType)
{
    QRegExp regEx = QRegExp(QStringLiteral("(.*)\\b(%1)\\b(.*)?").arg(tag));
    if (!rawLine().contains(regEx)) {
        return false;
    }

    // Support also notes where the token is at the end
    QString txt = regEx.cap(3).isEmpty() ? regEx.cap(1) : regEx.cap(3);
    // Remove possible comment char from both ends in a lazy way. So,
    // something too much could be gone but guess it's OK
    txt.remove(QRegExp(QStringLiteral("^\\W*")));
    txt.remove(QRegExp(QStringLiteral("\\W*$")));
    addNode(nodeType, regEx.cap(2).at(0) + QStringLiteral(": ") + txt, m_lineNumber);

    return true;
}


void ProgramParser::removeStrings()
{
    removeDoubleQuotedStrings();
}


void ProgramParser::removeDoubleQuotedStrings()
{
    // Thanks to https://stackoverflow.com/a/5696141
    m_line.remove(QRegExp(QStringLiteral("\"[^\"\\\\]*(?:\\\\.[^\"\\\\]*)*\"")));
    // Not sure if complete deletion is best or keeping a dummy
//     m_line.replace(QRegExp(QStringLiteral("\"[^\"\\\\]*(?:\\\\.[^\"\\\\]*)*\"")), QStringLiteral("X"));
}


void ProgramParser::removeSingleQuotedStrings()
{
    // Thanks to https://stackoverflow.com/a/5696141
    m_line.remove(QRegExp(QStringLiteral("\'[^\'\\\\]*(?:\\\\.[^\'\\\\]*)*\'")));
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
    static QRegExp rx(QStringLiteral("/\\*.*\\*/"));
    rx.setMinimal(true);

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
    p_rxHereDocOperator = QRegExp(hereDocOperator);

    // Build for each quote char an own RegExp
    QRegExp rx;
    for (int i = 0; i < quoteChars.size(); ++i) {
// qDebug() << "HereDocQuote:" << quoteChars.at(i);
        rx = QRegExp(QString(QStringLiteral("%1\\s?%2(.+)%2")).arg(hereDocOperator).arg(quoteChars.at(i)));
        rx.setMinimal(true);
        p_hereDocRxList << rx;
    }
    // Add at last a special RegEx for unquoted tokens
    // FIXME To avoid a couple of false catch, this regex may be more limited as useful
    // Problem was ";" in Perl and left shift a number "<< 20"
    rx = QRegExp(QString(QStringLiteral("%1\\s?([A-Za-z][^\\s;]*)")).arg(hereDocOperator));
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
        for (QRegExp rx : qAsConst(p_hereDocRxList)) {
            indexIn = rx.lastIndexIn(m_niceLine);
            if (indexIn < 0) {
                continue;
            }
            if (indexIn > lastIndexIn) {
                lastIndexIn = indexIn;
                token = rx.cap(1).trimmed();
// qDebug() << "TOKEN:" << token << rx.captureCount();
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

    if (node) {
        if (!node->text(2).contains(QStringLiteral("-1"))) {
            // Ignore this block
            if (p_nestingFoo < 0) {
                p_nestingFoo = p_nestingStack.size() + 1;
            }
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
        p_parentNode->setText(2, QString::number(lineNumber(), 10));
    }

    p_parentNode = p_parentNode->parent();
}


void ProgramParser::clearNesting()
{
    p_nestingFoo = -1;
    p_nestingStack.clear();
}


void ProgramParser::addNode(const int nodeType, const QString &text, const int lineNumber)
{
    QTreeWidgetItem *node = nullptr;

    if (m_detachedNodeTypes.contains(nodeType)) {
        node = new QTreeWidgetItem(p_rootNodes.value(nodeType), nodeType);
        setNodeProperties(node, nodeType, text, lineNumber);
        return;
    }

    if (!nodeTypeIsWanted(nodeType)) {
        return;
    }
    if (p_nestingFoo > -1) {
        return;
    }

    if (p_viewTree->isChecked()) {
        if (!p_parentNode || p_nestingStack.isEmpty()) {
            p_parentNode = p_rootNodes.value(nodeType);
        }
        if (m_nonBlockElements.contains(p_parentNode->type()) && p_parentNode->type() != nodeType) {
            p_parentNode = p_rootNodes.value(nodeType);
        }
        node = new QTreeWidgetItem(p_parentNode, nodeType);

    } else {
        node = new QTreeWidgetItem(p_indexTree, nodeType);
    }

    setNodeProperties(node, nodeType, text, lineNumber);

    if (m_nonBlockElements.contains(nodeType)) {
        node->setText(2, QString::number(lineNumber, 10));
    }
}


void ProgramParser::addNodeToScope(const QString &scope, const int scopeType, const int nodeType, const QString &text, const int lineNumber)
{
    if (!nodeTypeIsWanted(nodeType)) {
        return;
    }

    if (scope.isEmpty()) {
        addNode(nodeType, text, lineNumber);
        return;
    }

    QTreeWidgetItem *scopeRoot = p_scopeRoots.value(scope, nullptr);

    if (!scopeRoot) {
        scopeRoot = new QTreeWidgetItem(p_indexTree, nodeType);
        setNodeProperties(scopeRoot, scopeType, scope, -1);
        p_scopeRoots.insert(scope, scopeRoot);
    }

    QTreeWidgetItem *node = new QTreeWidgetItem(scopeRoot, nodeType);
    setNodeProperties(node, nodeType, text, lineNumber);
}


// kate: space-indent on; indent-width 4; replace-tabs on;
