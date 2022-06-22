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


#ifndef INDEXVIEW_XMLTYPEPARSER_CLASS_H
#define INDEXVIEW_XMLTYPEPARSER_CLASS_H

#include <QStack>

#include "parser.h"

/**
 * To crop the text shown in the node
 */
static const int Max_View_Lenght = 40;


class IndexView;
class KatePluginIndexView;

/**
 * The \p XmlTypeParser master class offer features for XML like files.
 *
 * @author loh.tar
 */

// Parsing XML is a welcome event for sarcastic comments
// https://stackoverflow.com/q/1732348
//
// This in mind I have tried here to use an approach without RegExp matching.
//
// I have tried to use Qt's XML processing, but it pointed out that I am to stupid
// or that these QXmlStreamReader class fit not this challenge. The doc says it, it
// must "well formed XML" but that is not to expect here.
// http://doc.qt.io/qt-5/xml-processing.html
//
// Further is XmlTypeParser not really correct because SGML is the original draft.
// See https://de.wikipedia.org/wiki/Standard_Generalized_Markup_Language (english page is less nice)
// But because Qt named his parser stuff this way and I had a doc book file around
// with sgml extension, I used this naming too.

/**
 * This XML parser is different from the other parser here. While the other ones are
 * "optimized" for one kind of document, with a limited set of pattern to find, is these
 * XML parser nearly a "true" XML parser. Which can load a set of pattern to use at runtime,
 * well almost. These loading is still hard coded but could be modified to read from some
 * file, I think. Lookt at @c loadDocType(), @c analyzeDocTypeTag() and @c DocType enum
 * So, there should be no need for a sub class to support some new file type even if the
 * documentation suggest so.
 * The starting point for this parser was @c ProgramParser so you may notice some similarities
 * or bad updated documentation...
 */
class XmlTypeParser : public Parser
{
    Q_OBJECT

public:
    XmlTypeParser(IndexView *view, const QString &docType);
   ~XmlTypeParser();

protected:
    QString version() override { return QStringLiteral("0.7, Jul 2022"); } ;
    QString author() override { return QStringLiteral("2022 loh.tar"); } ;

    /**
    * Overwrite these enum with an enhanced version in a sub class and ensure
    * to start with "FooNode = FirstNodeType,".
    * @see Parser::NodeType
    */
    enum NodeType {
        FixmeTodoNode,  // For  FIXME/TODO bookmarking
        BeginNode,      // For BEGIN bookmarking
        CommentNode,    // Yes, we collect comments like FIXME/TODO/BEGIN bookmarking
        RootNode,       // The root of our tree when..in tree view mode
        FirstNodeType   // Ensure to keep it here at the last place
    };

    // FIXME Not (really) used, guess no need for this
    enum EndTag {
        No,
        Optional,
        Required
    };

    struct TagType
    {
        TagType() {};
        TagType(int nt, int ett) : nodeType(nt), endTagType(ett) {};

        int nodeType = -1;
        int endTagType = EndTag::Required;
    };

    /**
     * @param tag The tag name as them occur in some document, e.g. h1 or section
     * @param iconType The icon type name defined by @c IconCollection::IconType
     * @return The number of the new added node type
     */
    int registerTag(const QString &tag, IconCollection::IconType iconType);

    bool tagIsKnown() { return p_currentTag.nodeType != -1; };
    bool tagIsKnownEndTag() { return p_currentTagIsEndTagOfType != -1; };
    bool tagIsComment() { return p_currentTagIsComment; };
    int  tagNodeType() { return p_currentTag.nodeType; };
    int  tagNodeTypeEndTag() { return p_currentTagIsEndTagOfType; };

    /**
     * This function iterate with each call over the document and uses Parser::appendNextLine()
     * to deliver each tag on @c m_line in @c m_tag with removed angle brackets and the
     * content in @c m_tagContent. The content of @c m_tag is anything what looks like a tag independent
     * what some parser class knows or is looking for or was set by @c registerTag()
     * @return true when anything is found what looked like a tag, false when end of document is reached
     */
    bool nextTag();

private:
    /**
     * This helper function is called from parseDocument() and add FIXME/TODO and BEGIN nodes.
     * @param rxType is used in a RegExp
     * @param nodeType should fitting to rxType
     */
    bool addSpecialCommentNode(const QString &rxType, const int nodeType);

    /**
     * This helper function is called in nextTag() and take care not to lost partial found
     * results when a tag span over more than one line. After that care taking is called
     * Parser::appendNextLine. There should be no need to use this function elsewhere.
     * @return Parser::appendNextLine
     */
    bool nextLine(int &anglePosition, bool alwaysCollect);

    /**
     * This helper function is called in nextTag() to find the next given @p angle in @c m_line.
     * While we search in @c m_line we take care of escaped char or quoted strings to avoid false
     * positives. When this function returns true is @c p_currCharIndex placed on the @p angle
     * position in @c m_line.
     * @return true when found, false when not
     */
    bool findNextAngle(QLatin1Char angle);

    /**
     * This helper function is called in nextTag() to build the needed final result between two
     * char positions.
     * @return string of interest
     */
    QString getStuffBetweenAngleBrackets(int fromAngle, int toAngle);

protected:
    virtual void prepareForParse() override;
    virtual void parseDocument() override;


    /**
     * This function will called in checkNesting() to increase the nesting level
     * but you can call it also direct when needed.
     */
    void beginOfBlock();

    /**
     * This function will called in checkNesting() to decrease the nesting level
     * but you can call it also direct when needed.
     */
    void endOfBlock();

    /**
     * This function can be called to reset the nesting.
     */
    void clearNesting();

    /**
     * @return true when node was added and false if not
     */
    bool addNode(const int nodeType, const QString &text, const int lineNumber, const int columnNumber);

    /**
     * A convenience function which will use @c m_tagContent, @c m_tagLine and @c m_tagColumn
     * to add the new node @param nodeType to the tree using the overload function.
     * @return true when node was added and false if not
     */
    bool addNode(const int nodeType);

    /**
     * Special function which uses @c m_attributeToUseForNodeText to set the text
     * for the new node.
     */
    bool addNodeWithTextFromAttribute(const int nodeType);

    /**
     * This helper replaces HTML entities like &amp; in @param text when @c p_betterConvertTagContent
     * is set to true.
     * @return the converted text cropped to @c Max_View_Lenght
     */
    QString convertTagContent(const QString &text);

    int parentNodeType() { return p_parentNode ? p_parentNode->type() : -1; };
    int nestingLevel() { return p_nestingStack.size(); };

    QSet<int>       m_nonBlockElements; // TODO guess can be removed, but is used in addNode()

    /**
     * Used e.g. by Head and Body html tags which has no content. But we need some
     * text to set the node text in our tree
     * @key is the nodeType, same as used in @c registerTag()
     * @value the text to be shown in the tree
     */
    QHash<int, QString>     m_nodeTextSpecial;

    /**
     * @key is the nodeType, same as used in @c registerTag()
     * @value A list of attribute names, first found is used
     */
    QHash<int, QStringList>     m_attributeToUseForNodeText;

    /**
     * Line number where the tag was found by @c nextTag.
     */
    int             m_tagLine;

    /**
     * The column number where @c nextTag has found the open angle bracket. The
     * position is calculated with current tab width in mind.
     */
    int             m_tagColumn;

    /**
     * The tag itself with its attributes but without angle brackets found by @c nextTag
     * e.g. '<foo bar="baz">' => 'foo bar="baz"'
     */
    QString         m_tag;

    /**
     * The tag element name in lowercase extracted from @c m_tag
     * e.g. '<FOO bar="baz">' => 'FOO bar="baz"' => 'foo'
     * This is used (or done) to excess @c p_tagTypes by its key. XML elements are not
     * case sensitive so far I know, but QHash keys are.
     */
    QString         m_tagName;

    /**
     * The tag content till next tag found by @c nextTag
     * e.g. '<foo>what ever<bar>' => 'what ever'
     */
    QString         m_tagContent;

    /**
     * Collect already found parts of interest on @c m_line when some text spans over
     * multiple lines. Filled by @c nextLine() and used at many places.
     */
    QString         m_partialResult;

private:
    enum DocType {
        Dummy,      // Fall back type when not really supported, just show comments (if found)
        Html,       // Also used for XHTML
        DocBook,
        Dtd,
        Xslt,
        Menu,
    };

    /**
     * This function is called from @c parseDocument and investigate the !doctype tag.
     * There is not much done but @c detectDocType called
     */
    void analyzeDocTypeTag();

    /**
     *
     * On success is @c loadDocType( called
     */
    void detectDocType(const QStringList &docType);

    /**
     * Load the doc type descriptions given by @p docType
     */
    void loadDocType(DocType docType);

    QAction                          *p_detachComments;

    QString                           p_loadedDocType;
    int                               p_currCharIndex; // Indicate position on m_line where we parse
    QHash<QString, TagType>           p_tagTypes;
    TagType                           p_currentTag; // Set by nextTag() and hold the result of p_tagTypes.value(m_tag)
    int                               p_currentTagIsEndTagOfType; // Set by nextTag() when current tag starts with / and hold the nodeType
    bool                              p_currentTagIsComment; // Set by nextTag() when current tag starts with !-- an end with --
    bool                              p_betterConvertTagContent; // Set by findNextAngle() and used in addNode()
    int                               p_nonWhiteSpaceRead; // Count in findNextAngle() and is compared to Max_View_Lenght

    QTreeWidgetItem*                  p_parentNode;
    QStack<QTreeWidgetItem*>          p_nestingStack;
    int                               p_nestingFoo; // FIXME Need better name. It's used to ignore nested content when parent is not wanted
};

#endif

// kate: space-indent on; indent-width 4; replace-tabs on;
