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


#ifndef INDEXVIEW_PROGRAMPARSER_CLASS_H
#define INDEXVIEW_PROGRAMPARSER_CLASS_H

#include <QStack>

#include "parser.h"

/**
 * To use with @c m_funcAtWork, like @c Me_At_Work, to inform lineIsGood() that
 * someone else takes over the responsibility that the line is good enough.
 */
#define Line_Is_Good QStringLiteral("LineIsGood")

class IndexView;
class KatePluginIndexView;

/**
 * The \p ProgramParser master class offer features for program like files.
 *
 * @author loh.tar
 */
class ProgramParser : public Parser
{

public:
    ProgramParser(QObject *view, KTextEditor::Document *doc);
   ~ProgramParser();

    virtual void parse() override;

protected:
    /**
    * Overwrite these enum with an enhanced version in a sub class and ensure
    * to start with "FooNode = FirstNodeType,".
    * @see Parser::NodeType
    */
    enum NodeType {
        FixmeTodoNode,
        BeginNode,      // Nope, nothing "begin" in our context, it's for //BEGIN tag
        FirstNodeType   // Ensure to keep it here at the last place
    };

    /**
    * This function iterate with each call over the document until no more data is left.
    * As long as lineIsGood() not returns "true" will lines to @c m_line appended
    * and @c m_lineNumber set. To do so is Parser::appendNextLine() called.
    * @warning This function clears @c m_line before doing its job
    * @returns true when successfull read, and false when no more data found
    */
    virtual bool nextInstruction();

    /**
     * This function validate @c m_line. By default is checked if the line is
     * terminated to fit C-Style code. So you may need to reimplent this function
     * to fit some needs.
     */
    virtual bool lineIsGood();

    /**
     * This function will called in nextInstruction() before the next line is processed.
     * So it is effectively called after a line was parsed, to update the nesting level when in
     * "tree view mode". To do so is a well working checkForBlocks() needed.
     * There should be no need to re-implement this function, therefore it's not virtual.
     */
    void checkNesting();

    /**
     * This function will called in nextInstruction() to set @c m_line and @c m_niceLine.
     * To achive this will first removeStrings() and then removeComment() called.
     */
    virtual void stripLine();

    /**
     * Helper function only used by stripLine() to add FIXME/TODO/BEGIN tags and
     * maybe some more.
     * @returns true when some tag was found
     */
    bool addCommentTagNode(const QString &tag, const int nodeType);

    /**
     * This function will called in stripLine() and remove by default all double
     * quoted strings by calling removeDoubleQuotedStrings(). Should there are
     * other quotings possible you should re-implement this function.
     * @note Strings can cause false detection of e.g. comments, so it's best to
     * eleminate them.
     */
    virtual void removeStrings();

    /**
     * This function will called by default in removeStrings() to do it's job.
     */
    void removeDoubleQuotedStrings();

    /**
     * This function is by default not called at all, but you should add it
     * to your own version of removeStrings() when single quoted strings are
     * possible.
     */
    void removeSingleQuotedStrings();

    /**
     * This function will called in stripLine() after removeStrings() and call
     * by default only removeTrailingSharpComment().
     * You should re-implement this function to fit your needs by add some of
     * the available remove comment functions or an custom function.
     * @note You should keep the following order when remove comments:
     * 1) Remove multi-line comment
     * 2) Remove in-line comment
     * 3) Remove trailing comment
     * 4) Remove heredocs and similar
     */
    virtual void removeComment();

    void removeMultiLineSlashStarComment();

    void removeInLineSlashStarComment();

    void removeTrailingSharpComment();
    void removeTrailingDoubleSlashComment();

    /**
     * Call this function in the parser constructor when the language support a
     * heredoc style syntax. The @p hereDocOperator is a RegExp and must match
     * against @c m_line. These and the quotes will later inserted to some
     * more complex RegExp to match agianst @c m_niceLine.
     * @note Not all heredoc syntax may could be catch with this approach
     * @see https://en.wikipedia.org/wiki/Here_document
     * @param hereDocOperator is is usually "<<" but sometimes "<<<" or "<<-?".
     * Ensure not to build capturing parentheses or later will cap(1) not work
     * @param quoteChars is a string which list all possible quotes. Typicaly is
     * the single ' or double quote " and the back tick ` used
     */
    void initHereDoc(const QString &hereDocOperator, const QString &quoteChars);

    /**
     * This function will remove heredocs according to previously
     * set @c p_rxHereDocOperator and @c quoteChars by initHereDoc()
     */
    void removeHereDoc();

    /**
     * This function will called in nextInstruction() and calculate p_bracesDelta.
     * By default are curly braces {} counted, so you may re-implement this
     * function when other chars used for blocks or overwrite with an empty
     * function to avoid unneeded checks.
     */
    virtual int checkForBlocks();

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
    void clearNesting();                // Introduced for Python

    /**
     * @see Parser::addNode()
     */
    virtual void addNode(const int nodeType, const QString &text, const int lineNumber, const int columnNumber = 0);

    /**
     * This function can be used instead of @c addNode to slightly bypass the normal
     * structured ordering of the tree. There will a new root node created, when not
     * already there, with the caption of @p scope and the type of @scopeType and
     * there the new node added.
     * @param scope the name of the scope, visible in the view as root node caption
     * @param scopeType type of the scope, in C++ is it NamespaceNode
     * @param nodeType the type of the new node, like header or function
     * @param text the caption of the new node, visible in the view
     * @param lineNumber the line where the pattern is located in the file
     * @param columnNumber the column where the pattern is located in the file
     * NOTE @p columnNumber is set but not reported (used) by any known sub class atm.
     */
    void addNodeToScope(const QString &scope, const int scopeType, const int nodeType, const QString &text, const int lineNumber, const int columnNumber = 0);

    /**
     * This function add a new root node with the caption of @p text to  the tree when
     * there is no current nesting situation. With nesting the node is add below the parent.
     * Without a nesting situation the new node is added to @c p_scopeRoots. Should such
     * scope already exist, nothing is done.
     * @param nodeType the type of the new node, like struct
     * @param text the caption of the new node, visible in the view
     * @param lineNumber the line where the pattern is located in the file
     * @param columnNumber the column where the pattern is located in the file
     * NOTE @p columnNumber is set but not reported (used) by any known sub class atm.
     *
     */
    void addScopeNode(const int nodeType, const QString &text, const int lineNumber, const int columnNumber = 0);

    int parentNodeType() { return p_parentNode ? p_parentNode->type() : -1; }; // Introduced for C++ function declarations
    int nestingLevel() { return p_nestingStack.size(); }; // Introduced for Tcl

    QSet<QString>   m_blockElements;
    QSet<int>       m_nonBlockElements;
    QSet<int>       m_nestableElements;

    /**
     * Simplified line, may handy to extract data.
     */
    QString         m_niceLine;

    /**
     * Current line number.
     */
    int             m_lineNumber;

private:
    QTreeWidgetItem*                  p_parentNode;
    QStack<QTreeWidgetItem*>          p_nestingStack;
    int                               p_bracesDelta;
    int                               p_nestingFoo; // FIXME Need better name. It's used to ignore nested content when parent is not wanted
    QRegularExpression                p_rxHereDocOperator;
    QList<QRegularExpression>         p_hereDocRxList;
    QHash<QString, QTreeWidgetItem*>  p_scopeRoots; // Introduced for C++ function definitions
};

#endif

// kate: space-indent on; indent-width 4; replace-tabs on;
