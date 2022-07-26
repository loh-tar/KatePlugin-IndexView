/*   This file is part of KatePlugin-IndexView
 *
 *   XmlTypeParser Class
 *   Copyright (C) 2022 loh.tar@googlemail.com
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
#include <QStringLiteral>

#include "index_view.h"

#include "parser.h"
#include "xml_type_parser.h"


XmlTypeParser::XmlTypeParser(IndexView *view, const QString &docType)
    : Parser(view, docType)
{
    useNestingOptions();

    using namespace IconCollection;
    registerViewOption(FixmeTodoNode, FixmeTodoIcon, QStringLiteral("FIXME/TODO"), i18n("Show FIXME/TODO"));
    registerViewOption(BeginNode, BeginIcon, QStringLiteral("BEGIN"), i18n("Show BEGIN"));
    registerViewOption(CommentNode, CommentIcon, QStringLiteral("Comments"), i18n("Show Comments"));

    p_detachComments = registerViewOptionModifier(CommentNode, QStringLiteral("DetachComments"), i18n("Detach Comments"));

    m_detachedNodeTypes << FixmeTodoNode << BeginNode;

    setNodeTypeIcon(RootNode, DocumentRootIcon);

    detectDocType({docType});
}


XmlTypeParser::~XmlTypeParser()
{
}


void XmlTypeParser::prepareForParse()
{
    m_line.clear();
    p_currCharIndex = 0;

    p_parentNode = nullptr;
    clearNesting();
    resetNesting();

    p_indexTree->setRootIsDecorated(1);

    if (p_detachComments->isChecked()) {
        m_detachedNodeTypes << CommentNode;
    } else {
        m_detachedNodeTypes.remove(CommentNode);
    }

    // Add the root node here keeps addNode() less complex
    // Using line number 0 will cause a jump to the top of the document when
    // clicked which make absolutely sense.
    QTreeWidgetItem *node = rootNode(RootNode);
    setNodeProperties(node, RootNode, i18n("Document"), 0);
    beginOfBlock();
}


void XmlTypeParser::parseDocument()
{
//     qDebug() << "XmlTypeParser::parseDocument";
    // Tag content of our interest is often spiked with inline tags which we not even know
    // and ignore. But we need to collect there content and use it for our known tags
    QString contentCollector;
    bool lastNodeNeedContent = false;
    bool newContentIsAvailable = false;

    auto collectUnusedContent = [this, &contentCollector, &lastNodeNeedContent, &newContentIsAvailable]() {
        if (!lastNodeNeedContent || p_nonWhiteSpaceRead < 1) {
            return;
        }
        contentCollector.append(QLatin1Char(' '));
        contentCollector.append(m_tagContent);
        newContentIsAvailable = true;
    };

    auto updateTextOnLastNode = [this, &contentCollector, &lastNodeNeedContent, &newContentIsAvailable]() {
        if (!lastNodeNeedContent) {
            return;
        }
        // Reset this hint early, ensure we don't miss this...
        lastNodeNeedContent = false;

        if (contentCollector.isEmpty()) {
             return;
        } else if (!newContentIsAvailable) {
            p_betterConvertTagContent = false;
            contentCollector.clear();
            return;
        }

        // Only update when the node was not already closed => content not related to this node
        if (lastNode()->data(0, EndLine).toInt() < 0) {
//             qDebug() << "UPDATE TEXT <" << lastNode()->text(0) << lastNode()->data(0, EndLine).toInt() << p_betterConvertTagContent;
            lastNode()->setText(0, convertTagContent(contentCollector));
//             qDebug() << "UPDATE TEXT >" << lastNode()->text(0);
        }
        // ...and that. Now all important stuff is reset, ready for next customer
        newContentIsAvailable = false;
        p_betterConvertTagContent = false;
        contentCollector.clear();
    };

    while (nextTag()) {
        if (m_tag.startsWith(QStringLiteral("!doctype"), Qt::CaseInsensitive)) {
                analyzeDocTypeTag();

        } else if (m_tag.endsWith(QStringLiteral("/"))) {
            if (m_attributeToUseForNodeText.contains(tagNodeType())) {
                addNodeWithTextFromAttribute(tagNodeType());
                continue;
            }
            // It's a so called empty-element, guess we can't do much useful here
            collectUnusedContent();

        } else if (tagIsComment()) {
            // Often are extra - = # chars around to let some comment looks nice in the code,
            // but this is bad for our needs => remove them
            static const QRegularExpression rx1(QStringLiteral("^![-=#\\ ]*"));
            static const QRegularExpression rx2(QStringLiteral("[-=#]*$"));
            m_tagContent.remove(rx1);
            m_tagContent.remove(rx2);
            // I have also seen empty comments, these are pointless, skip them!
            if (!m_tagContent.isEmpty()) {
                if (!addSpecialCommentNode(QStringLiteral("FIXME|TODO"), FixmeTodoNode)) {
                    if (!addSpecialCommentNode(QStringLiteral("BEGIN"), BeginNode)) {
                        addNode(CommentNode);
                    }
                }

                if (!p_detachComments->isChecked()) {
                    lastNode()->setData(0, EndLine, lineNumber());
                }
            }

        } else if (tagIsKnown()) {
            updateTextOnLastNode();
            if (m_nodeTextSpecial.contains(tagNodeType())) {
                m_tagContent = m_nodeTextSpecial.value(tagNodeType());
                addNode(tagNodeType());

            } else if (m_attributeToUseForNodeText.contains(tagNodeType())) {
                addNodeWithTextFromAttribute(tagNodeType());

            } else {
                // Last line of defense to ensure we have some text in our new node
                if (p_nonWhiteSpaceRead < 1) {
                    m_tagContent = m_tagName;
                } else {
                    contentCollector = m_tagContent;
                }

                if (addNode(tagNodeType()) && lastNode()->text(0).size() < Max_View_Lenght) {
                    lastNodeNeedContent = true;
                } else {
                    contentCollector.clear();
                }
            }

            beginOfBlock();

        } else if (tagIsKnownEndTag()) {
            if (lastNode() && tagNodeTypeEndTag() == lastNode()->type()) {
                updateTextOnLastNode();
            }
            endOfBlock(); // Important to call after updateTextOnLastNode() !

        } else {
            collectUnusedContent();
        }
    }

    // It could be that now is some text pending, use it
    updateTextOnLastNode();
    // Ensure our root node got EndLine set
    endOfBlock();

    // When we have only found comments looks the empty added root node odd,
    // even more because they is not on top but below the comment root node
    QTreeWidgetItem *rootNode = p_rootNodes.value(RootNode);
    if (rootNode->childCount() < 1) {
        rootNode->setHidden(true);
    }
}


bool XmlTypeParser::addSpecialCommentNode(const QString &rxType, const int nodeType)
{
    // FIXME ? No static here, but could be done when we again check what kind of special it is
    const QRegularExpression regEx = QRegularExpression(QStringLiteral("(.*)\\b(%1)\\b(.*)?").arg(rxType));
    const QRegularExpressionMatch rxMatch = regEx.match(m_tagContent);

    if (!rxMatch.hasMatch()) {
        return false;
    }

    // Support also notes where the token is at the end or in the middle of nowhere
    m_tagContent = rxMatch.captured(1) + rxMatch.captured(3);
    m_tagContent = m_tagContent.simplified(); // Now we are extra pedantic
    addNode(nodeType);

    return true;
}


bool XmlTypeParser::nextLine(int &anglePosition, bool alwaysCollect)
{
    // HINT: These alwaysCollect is set to !needCloseAngle or !needOpenAngle what
    // looks strange. We do this to indicate that we are interested in some partial result
    // or even not.

    if (p_currCharIndex < m_line.size()) {
        // Nothing todo, there is some stuff left on line
        return true;
    }

    if (anglePosition > -1) {
        // We are half way done and need to remember the found partial result
        m_partialResult = m_line.mid(anglePosition);
        anglePosition = -1; // Mark referenced angel as no longer valid
    } else if (alwaysCollect) {
        m_partialResult += QLatin1Char(' ') + m_line;
    }

    m_line.clear();
    p_currCharIndex = 0;

    return Parser::appendNextLine();
}


bool XmlTypeParser::findNextAngle(QLatin1Char angle)
{
    int ampersandFound = -1;
    int semicolonFound = -1;

    for (p_currCharIndex = qMax(p_currCharIndex, 0); p_currCharIndex < m_line.size(); ++p_currCharIndex) {
        if (m_line.at(p_currCharIndex) == angle) {
            return true;
        } else if (m_line.at(p_currCharIndex) == QLatin1Char('\\')) {
            // Skip next char because it is escaped
            ++p_currCharIndex;
        } else if (m_line.at(p_currCharIndex) == QLatin1Char('&')) {
            ampersandFound = p_currCharIndex;
        } else if (m_line.at(p_currCharIndex) == QLatin1Char(';')) {
            semicolonFound = p_currCharIndex;
        } else if (!m_line.at(p_currCharIndex).isSpace()) {
            ++p_nonWhiteSpaceRead;
        }

        if (p_currCharIndex == semicolonFound && ampersandFound > -1 && semicolonFound > ampersandFound) {
            p_betterConvertTagContent = true;
            // Let us adjust our white space counter. That works only so nice because we check
            // for p_currCharIndex == semicolonFound too and avoid with each loop this adjustment
            p_nonWhiteSpaceRead -= semicolonFound - ampersandFound;
        }
    }

    return false;
}


QString XmlTypeParser::getStuffBetweenAngleBrackets(int fromAngle, int toAngle)
{
    QString content;

    // We have a winner? Puh, there are many cases to fumble with
    if (m_partialResult.isEmpty() && toAngle > -1) {
        content = m_line.mid(fromAngle, toAngle - fromAngle);
    } else if (!m_partialResult.isEmpty() && toAngle > -1 && fromAngle > -1) {
        content = m_partialResult + QLatin1Char(' ') + m_line.mid(fromAngle, toAngle - fromAngle);
    } else if (!m_partialResult.isEmpty() && toAngle > -1 && fromAngle < 0) {
        content = m_partialResult + QLatin1Char(' ') + m_line.mid(0, toAngle);
    } else if (fromAngle > -1) {
        content = m_partialResult + m_line.left(fromAngle);
    } else {
        content = m_partialResult;
    }
//     qDebug() << "XmlTypeParser::getStuffBetweenAngleBrackets" << fromAngle << toAngle << m_partialResult;

    return content;
}


bool XmlTypeParser::nextTag()
{
    // Ensure we report no nonsense
    p_currentTagIsComment = false;
    p_currentTagIsEndTagOfType = -1;

    int openAngle = -1;
    int closeAngle = -1;
    bool needOpenAngle = true;
    bool needCloseAngle = true;
    bool anglePairIsValid = false;

    m_tagColumn = 0;
    m_tag.clear();
    m_tagContent.clear();
    m_partialResult.clear();

    while (!anglePairIsValid) {
        if (!nextLine(openAngle, !needOpenAngle)) {
//             qDebug() << "EXIT while search tag" << lineNumber();
            return false;
        }

        if (needOpenAngle) {
            if (findNextAngle(QLatin1Char('<'))) {
                openAngle = p_currCharIndex + 1; // Add 1, we don't want the < itself
                m_tagColumn = p_currCharIndex;
                m_tagLine = Parser::lineNumber();
                ++p_currCharIndex;
                needOpenAngle = false;
            }
        }

        if (needCloseAngle) {
            if (findNextAngle(QLatin1Char('>'))) {
                closeAngle = p_currCharIndex;
                ++p_currCharIndex;
                needCloseAngle = false;
            }
        }

        // Do we have some result?
        if (!needOpenAngle && !needCloseAngle) {
            // The reward for all the effort
            m_tag = getStuffBetweenAngleBrackets(openAngle, closeAngle).simplified();
            // Check if current tag is a (valid) comment
            if (!m_tag.startsWith(QStringLiteral("!--"))) {
                anglePairIsValid = true;
            } else if (m_tag.endsWith(QStringLiteral("--"))) {
                anglePairIsValid = true;
                p_currentTagIsComment = true;
                // OK, now we know it's a comment, we need the tag itself as the tag content
                m_tagContent = m_tag;
//                 qDebug() << "XmlTypeParser:: COMMENT:" << "line:" << m_tagLine << "column:" << m_tagColumn << m_tag;
                return true; // We have all we need!
            } else {
                // We are not done!
                needCloseAngle = true;
            }
        }
    }

    // Now, that we have a valid tag which is not a comment, examine what it is
    int spacePos = m_tag.indexOf(QLatin1Char(' '));
    m_tagName = m_tag.mid(0, spacePos).toLower();
    p_currentTag = p_tagTypes.value(m_tagName);

    if (m_tagName.startsWith(QLatin1Char('/'))) {
        p_currentTagIsEndTagOfType = p_tagTypes.value(m_tagName.mid(1)).nodeType;
    }
//     qDebug() << "XmlTypeParser:: FOUND TAG:" << "line:" << m_tagLine << "column:" << m_tagColumn << m_tagName << m_tag;

    // What now is missing, is the tag content, so we do all again similar hm...
    openAngle = -1;          // Reset, we search the next open angle
    needOpenAngle = true;
    closeAngle += 1;         // Keep it but move one, we don't want the angel itself as our content
    m_partialResult.clear();
    p_nonWhiteSpaceRead = 0; // Now we need..no, want to check for read useful chars to be sure we have enough

    while (needOpenAngle) {
        if (!nextLine(closeAngle, !needCloseAngle)) {
//             qDebug() << "EXIT while search content" << lineNumber();
            return false;
        }

        // Avoid unneeded data collection...
        if (p_nonWhiteSpaceRead > Max_View_Lenght) {
            // ...we have enough stuff for our purposes
            m_tagContent = m_partialResult;
            break;
        }

        if (findNextAngle(QLatin1Char('<'))) {
            openAngle = p_currCharIndex; // No add 1 this time, we don't want the < itself
            needOpenAngle = false;
        }

        // Do we have some result?
        if (!needOpenAngle && !needCloseAngle) {
            // The reward for all the effort
            m_tagContent = getStuffBetweenAngleBrackets(closeAngle, openAngle);
        }
    }

//     qDebug() << "XmlTypeParser:: NEW TAG:" << "line:" << m_tagLine << "column:" << m_tagColumn << m_tagName << m_tag << "content:" << m_tagContent;

    return true;
}


void XmlTypeParser::beginOfBlock()
{
    ++p_nestingLevel;

    QTreeWidgetItem *node = lastNode();

    if (!p_nestingStack.isEmpty()) {
        if (p_nestingStack.top() == node) {
            // Ignore this block
            if (p_nestingFoo < 0) {
                p_nestingFoo = p_nestingStack.size() + 1;
            }
        }
    }

    if (node && node->data(0, EndLine).toInt() != -1) {
        // Ignore this block
        if (p_nestingFoo < 0) {
            p_nestingFoo = p_nestingStack.size() + 1;
        }
    }

    p_parentNode = node;
    p_nestingStack.push(node);
}


void XmlTypeParser::endOfBlock()
{
    --p_nestingLevel;

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
        p_parentNode->setData(0, EndLine, lineNumber());
    }

    p_parentNode = p_parentNode->parent();
}


void XmlTypeParser::clearNesting()
{
    p_nestingFoo = -1;
    p_nestingStack.clear();
}


bool XmlTypeParser::addNode(const int nodeType)
{
    return addNode(nodeType, m_tagContent, m_tagLine, m_tagColumn);
}

bool XmlTypeParser::addNodeWithTextFromAttribute(const int nodeType)
{
    QRegularExpressionMatch rxMatch;

    for (QStringListIterator i(m_attributeToUseForNodeText.value(nodeType)); i.hasNext(); ) {
        const QRegularExpression regEx = QRegularExpression(QStringLiteral(R"(\s%1\=\"([^"]+)\")").arg(i.next()));

        if (!m_tag.contains(regEx, &rxMatch)) {
            continue;
        }
        return addNode(nodeType, rxMatch.captured(1) , m_tagLine, m_tagColumn);
    }

    // What now? We have a tag but no fitting attribute. Hm...
    return addNode(nodeType, i18n("Error: attribute not found") , m_tagLine, m_tagColumn);
}

bool XmlTypeParser::addNode(const int nodeType, const QString &text, const int lineNumber, const int columnNumber)
{
    QTreeWidgetItem *node = nullptr;
//     qDebug() << "nesting" << p_nestingFoo << p_nestingStack.size();

    if (m_detachedNodeTypes.contains(nodeType)) {
        node = new QTreeWidgetItem(rootNode(nodeType), nodeType);
        setNodeProperties(node, nodeType, convertTagContent(text), lineNumber, columnNumber);
        return true;
    }

// Disabled because we have no options for our nodes. Keep it only...as reminder
//     if (!nodeTypeIsWanted(nodeType)) {
//         return false;
//     }

    p_maxNesting = qMax(p_maxNesting, p_nestingLevel);

    if (p_nestingStack.size() > p_nestingAllowed) {
        return false;
    }
    if (p_nestingFoo > -1) {
        return false;
    }

    if (!p_parentNode || p_nestingStack.isEmpty()) {
        p_parentNode = rootNode(nodeType);
    }
    if (m_nonBlockElements.contains(p_parentNode->type()) && p_parentNode->type() != nodeType) {
        p_parentNode = p_rootNodes.value(nodeType);
    }
    node = new QTreeWidgetItem(p_parentNode, nodeType);

    setNodeProperties(node, nodeType, convertTagContent(text), lineNumber, columnNumber);

    if (m_nonBlockElements.contains(nodeType)) {
        node->setData(0, EndLine, lineNumber);
    }

    return true;
}


static QString convertText(QString text)
{
// This approach works nicely, but is expensive... (parsing time was doubled!)
//     QTextEdit converter;
//     converter.setHtml(QStringLiteral("<p>%1</p>").arg(text));
//     return converter.toPlainText().simplified();

// ...so we try something simpler
    text = text.replace(QStringLiteral("&nbsp;"), QStringLiteral(" "));
    text = text.replace(QStringLiteral("&amp;"), QStringLiteral("&"));
    text = text.replace(QStringLiteral("&lt;"), QStringLiteral("<"));
    text = text.replace(QStringLiteral("&gt;"), QStringLiteral(">"));
    text = text.replace(QStringLiteral("&apos;"), QStringLiteral("'"));
    text = text.replace(QStringLiteral("&quot;"), QStringLiteral("\""));
//     text = text.replace(QStringLiteral("&;"), QStringLiteral(""));

    return text.simplified();
}

QString XmlTypeParser::convertTagContent(const QString &text)
{
    if (p_betterConvertTagContent) {
        // We convert first all text and then check for Max_View_Lenght
        // otherwise we would need to deal with cut entities e.g. &amp; => &am
        QString plainText = convertText(text);
        if (plainText.size() > Max_View_Lenght) {
            return plainText.left(Max_View_Lenght) + QStringLiteral("…");
        }
        return plainText;
    }

    QString simplifiedText = text.simplified();

    if (simplifiedText.size() > Max_View_Lenght) {
        return simplifiedText.left(Max_View_Lenght) + QStringLiteral("…");
    }

    return simplifiedText;
}


int XmlTypeParser::registerTag(const QString &tag, IconCollection::IconType iconType)
{
    int nodeType = FirstNodeType + p_tagTypes.size();
    p_tagTypes.insert(tag.toLower(), TagType(nodeType, EndTag::Required));
    p_nodeTypes.insert(nodeType, NodeTypeStruct(tag.toLower(), IconCollection::getIcon(iconType)));

    return nodeType;
}


void XmlTypeParser::analyzeDocTypeTag()
{
    // FIXME These doctype trick is fine so far, but e.g. openbox uses for their config xml files
    // (see /etc/xdg/openbox/menu.xml) a "header" like this
    //   <?xml version="1.0" encoding="UTF-8"?>
    //   <openbox_menu xmlns="http://openbox.org/3.4/menu">
    // While https://en.wikipedia.org/wiki/XML notice the first line as "XML declaration" is
    // the second line not mentioned. But later is a "Schema" example which looks similar.
    // Whatever: I guess we can't check for each thinkable doc type and add special stuff as currently
    // done. On the long run we some other solution, like read from config files *sigh*

    // FIXME Is this check a little bit too simple? e.g. XHTML fit html, well currently nice but..
    if (!p_loadedDocType.isEmpty() && m_tag.contains(p_loadedDocType, Qt::CaseInsensitive)) {
        return;
    }

    // Try to detect the document from it's content..
    // This is not the finest art, but seems to be good enough(?)
    detectDocType(m_tag.split(QLatin1Char(' '), Qt::SkipEmptyParts));
}


void XmlTypeParser::detectDocType(const QStringList &docType)
{
    // We need to "load the setup", ensure all is clean...
    p_tagTypes.clear();
    m_nodeTextSpecial.clear();
    m_attributeToUseForNodeText.clear();
    // ..reset intern auto color counter so the colors are always the same between doc type changes
    IconCollection::getIcon(-1);

    if (   docType.contains(QStringLiteral("\"language.dtd\""), Qt::CaseInsensitive)
        || docType.contains(QStringLiteral("DTD"), Qt::CaseInsensitive)
    ) {
        loadDocType(DocType::Dtd);

    } else if (docType.contains(QStringLiteral("html"), Qt::CaseInsensitive)
            || docType.contains(QStringLiteral("XHTML"), Qt::CaseInsensitive)
    ) {
        loadDocType(DocType::Html);

    } else if (docType.contains(QStringLiteral("DocBook"), Qt::CaseInsensitive)
            // The only SGML file I have seen was a doc book
            || docType.contains(QStringLiteral("SGML"), Qt::CaseSensitive)
    ) {
        loadDocType(DocType::DocBook);

    } else if (docType.contains(QStringLiteral("xslt"), Qt::CaseSensitive)) {
        loadDocType(DocType::Xslt);

    } else if (docType.contains(QStringLiteral("Menu"), Qt::CaseSensitive)) {
        loadDocType(DocType::Menu);

    } else {
        // Guess we was called from ctor, so we keep that as Dummy string hm...
        p_loadedDocType = docType.at(0);
    }
}


void XmlTypeParser::loadDocType(DocType docType)
{
    int nodeType;
    using namespace IconCollection;

    switch (docType) {
        case Html:
            p_loadedDocType = QStringLiteral("html");
            nodeType = registerTag(QStringLiteral("Head"), AutoColor3Icon);
            // No i18n translations here, that would be pointless
            m_nodeTextSpecial.insert(nodeType, QStringLiteral("Head"));
            registerTag(QStringLiteral("TITLE"), AutoColor1Icon);
            nodeType = registerTag(QStringLiteral("Body"), AutoColor3Icon);
            m_nodeTextSpecial.insert(nodeType, QStringLiteral("Body"));
            nodeType = registerTag(QStringLiteral("Article"), AutoColor2Icon);
            m_nodeTextSpecial.insert(nodeType, QStringLiteral("Article"));
            registerTag(QStringLiteral("H1"), Head1Icon);
            registerTag(QStringLiteral("H2"), Head2Icon);
            registerTag(QStringLiteral("H3"), Head3Icon);
            registerTag(QStringLiteral("H4"), Head4Icon);
            registerTag(QStringLiteral("H5"), Head5Icon);
            registerTag(QStringLiteral("H6"), Head6Icon);
            registerTag(QStringLiteral("P"), ParaIcon);
            break;

        case DocBook:
            p_loadedDocType = QStringLiteral("DocBook");
            registerTag(QStringLiteral("info"), AutoColor2Icon);
            registerTag(QStringLiteral("refmeta"), AutoColor2Icon);
            registerTag(QStringLiteral("RefnameDiv"), AutoColor3Icon);
            registerTag(QStringLiteral("RefsynopsisDiv"), AutoColor3Icon);
            registerTag(QStringLiteral("Sect1"), AutoColor3Icon);
            registerTag(QStringLiteral("Section"), AutoColor2Icon);
            registerTag(QStringLiteral("RefSect1"), AutoColor3Icon);
            registerTag(QStringLiteral("RefSect2"), AutoColor2Icon);
            registerTag(QStringLiteral("RefSection"), AutoColor2Icon);
            registerTag(QStringLiteral("varlistentry"), AutoColor2Icon);
            registerTag(QStringLiteral("para"), AutoColor1Icon);
            break;

        case Dtd:
            p_loadedDocType = QStringLiteral("language.dtd");
            // Nothing todo! But we support comments
            break;

        case Xslt:
            // https://www.w3schools.com/xml/xsl_elementref.asp
            p_loadedDocType = QStringLiteral("Xslt");
            nodeType = registerTag(QStringLiteral("xsl:template"), AutoColor3Icon);
            m_attributeToUseForNodeText.insert(nodeType, {QStringLiteral("match"), QStringLiteral("name")});
            nodeType = registerTag(QStringLiteral("xsl:call-template"), AutoColor2Icon);
            m_attributeToUseForNodeText.insert(nodeType, {QStringLiteral("name")});
            nodeType = registerTag(QStringLiteral("xsl:with-param"), AutoColor1Icon);
            m_attributeToUseForNodeText.insert(nodeType, {QStringLiteral("select"), QStringLiteral("name")});
            nodeType = registerTag(QStringLiteral("xsl:param"), AutoColor1Icon);
            m_attributeToUseForNodeText.insert(nodeType, {QStringLiteral("name")});
            nodeType = registerTag(QStringLiteral("xsl:variable"), AutoColor1Icon);
            m_attributeToUseForNodeText.insert(nodeType, {QStringLiteral("name")});
            nodeType = registerTag(QStringLiteral("xsl:when"), AutoColor2Icon);
            m_attributeToUseForNodeText.insert(nodeType, {QStringLiteral("test")});
            nodeType = registerTag(QStringLiteral("xsl:otherwise"), AutoColor2Icon);
            //  no  m_attributeToUseForNodeText here !
            break;

        case Menu:
            // Only tested with /etc/xdg/menus/applications.menu
            p_loadedDocType = QStringLiteral("Menu");
            registerTag(QStringLiteral("Menu"), AutoColor3Icon);
            registerTag(QStringLiteral("Directory"), AutoColor2Icon);

        case Dummy:
        default:
            p_loadedDocType = QStringLiteral("Dummy");
            // Nothing todo! But we support comments
            break;
    }
}

// kate: space-indent on; indent-width 4; replace-tabs on;
