/*   This file is part of KatePlugin-IndexView
 *
 *   Parser Class
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


#ifndef INDEXVIEW_PARSER_CLASS_H
#define INDEXVIEW_PARSER_CLASS_H

#include <QAction>
#include <QElapsedTimer>
#include <QMenu>
#include <QObject>
#include <QQueue>
#include <QString>
#include <QTreeWidget>
#include <QTreeWidgetItem>

#include <KTextEditor/Document>

#include "icon_collection.h"


// FIXME Idea for a better solution?
// add tests for other compiler than gcc
// https://stackoverflow.com/a/4384825
// #ifdef foo
/**
 * To use with @c m_funcAtWork
 */
#define Me_At_Work QLatin1String(__PRETTY_FUNCTION__)
// #elif bar Visual C++
// #define Me_At_Work QLatin1String(__FUNCSIG__)
// #endif


class IndexView;
class KatePluginIndexView;

/**
 * The @c Parser class must be the base for all @c IndexView parser.
 * However it may not recommended to use this class as a direct parent for new
 * written parser. There are (currently two) derivated master parser available
 * which offer a couple of helpful features.
 * @see DocumentParser, ProgramParser
 * @author loh.tar
 */
class Parser : public QObject
{
    // Each parser must use this macro due to use of metaObject()->className()
    // in KatePluginIndexView::configPage()
    Q_OBJECT

    friend class DummyParser;
    friend class DocumentParser;
    friend class ProgramParser;

public:
    Parser(IndexView *view);
   ~Parser();

   /**
    * Each parser must implement this function to give a hint at the config
    * about page.
    * @return a version number
    */
   virtual QString version() = 0;

   /**
    * Each parser must implement this function to give a hint at the config
    * about page.
    * @return a list of authors separated by @c \n with leading year
    */
   virtual QString author() = 0;

    /**
    * Create a new parser object fitting to given @p type.
    * @param type the document type, e.g. "C++"
    * @param view the related index view
    * @return nullptr when @p type is not supported
    * @see create(), parse()
    */
    static Parser *create(const QString &type, IndexView *view);

    /**
    * This is the main access function to parse the document. These will call
    * prepareForParse() and parseDocument(). Before and after that will done
    * some janitor task like clear the tree and update the context menu to hide
    * unneeded options. Only some master classes may need to implemented an own
    * version to init some special variables or to do other special treatment.
    * Ensure in that case to call Parser::parse() at some point.
    */
    virtual void parse();

    /**
     * Call this function to inform the parser to use a different document.
     */
    void docChanged();

    /**
     * Get the context menu which hold all view settings.
     */
    QMenu *contextMenu() { return &p_menu; };

protected:
    /**
    * Overwrite these enum with an enhanced version in a sub class and use them
    * in every function call where a @c nodeType as parameter is needed to keep
    * the code good readable.
    */
    enum NodeType {
        DummyNode
    };

    /**
     * This function increment with each call the index @c p_lineNumber to access
     * the document lines. There is very rare a need to use this function. They
     * exist only for cases where a non master class function need to iterate over
     * the document.
     * @return false when @c p_lineNumber gets bigger than documentSize()
     */
    bool incrementLineNumber();

    /**
    * This function give access to the unstripped last read document line. Calling
    * this will not change @c p_lineNumber.
    * @param offset is added to @c p_lineNumber to index the line of the document
    * @return the current line from the document indexed by @c p_lineNumber + @p offset.
    * If the result is less 0 or bigger than documentSize(), an empty string is returned.
    */
    QString rawLine(int offset = 0) const;

    /**
    * This function iterate with each call over the document and append the line
    * which is indexed by @c p_lineNumber to @c m_line.
    * This function also checks @c m_runTime and calls qApp->processEvents() if too
    * much time is gone to keep the editing responsive.
    * @return false when no line was left
    */
    virtual bool appendNextLine();

    /**
     * @return the last read line number by appendNextLine()
     */
    int lineNumber() const { return p_lineNumber - 1; }

    /**
    * This is the beef and must be implemented by each derivated parser class.
    */
    virtual void parseDocument() = 0;

    /**
    * This function is called by parse() just before parseDocument() and offer
    * the possibilities to do some initialization in a master class after the
    * janitor initialization task is done, like clear the tree. By default there
    * will be only the root decoration set according to checked @c p_viewTree.
    */
    virtual void prepareForParse();

    /**
     * This function return the root node of the given @p nodeType holded in @c p_rootNodes.
     * If such node not exist, is a new node created, added to @c p_rootNodes and returned.
     * @param nodeType of type NodeType
     * @return root node of asked type
     */
    QTreeWidgetItem *rootNode(int nodeType);

    /**
     * Register an icon to use for @p nodeType.
     * See @c IconCollection::getIcon for @p size and @p qtGlobalColorEnum.
     */
    void setNodeTypeIcon(const int nodeType, const int size, const int qtGlobalColorEnum = -1);

    /**
    * Add a new view option to the context menu to toggle the visibility of the
    * node in question.
    * @note There should be no need to store and use the returned QAction,
    * @param nodeType should be some enum of the sub class NodeType
    * @param iconType is the icon type to display in the menu option and the tree node
    * @param name is used to store the setting, therefore don't translate this
    * @param caption is visible in the menu and should be translated
    * @return the new added action
    */
    QAction *registerViewOption(const int nodeType, const IconCollection::IconType iconType, const QString &name, const QString &caption);

    /**
    * Add a new view option to the context menu to modify the look of some node,
    * e.g. "Show Parameter"
    * @note As opposed to registerViewOption() should you store and use the returned
    *       QAction later in parseDocument().
    * @param nodeType is the option to modify
    * @param name is used to store the setting, therefore don't translate this
    * @param caption is visible in the menu and should be translated
    * @return the new added action
    */
    QAction *registerViewOptionModifier(const int nodeType, const QString &name, const QString &caption);

    /**
    * Add a new view option to the context menu to modify the general look of the
    * displayed index view, e.g. "Show Sorted". This function should only be used
    * in a master class where the view option added this way is checked in addNode().
    * @param name is used to store the setting, therefore don't translate this
    * @param caption is visible in the menu and should be translated
    * @return the new added action
    */
    QAction *addViewOption(const QString &name, const QString &caption);

    /**
     * Add a separator to the context menu.
     */
    void addViewOptionSeparator();

    /**
    * Add a dependency to a view option. When the dependency is not checked will
    * the dependent option out grayed. This is achieved by adding an entry to
    * @c p_modifierOptions.
    * @note This will automatically done by registerViewOptionModifier(), so using
    * this function is only needed in rare cases, like in RubyParser::RubyParser().
    * @param dependentNodeType which is dependent from @p dependencyNodeType
    * @param dependencyNodeType is the dependency for @p dependentNodeType
    */
    void addViewOptionDependency(int dependentNodeType, int dependencyNodeType);

    /**
    * Add a new node to the index view with respect of current view settings.
    * Each master class must implement this function.
    * @note: A "not enabled" option should be treated as "not checked" by addNode()
    *        @see nodeTypeIsWanted()
    * @param nodeType the type of the new node, like header or function
    * @param text the caption of the new node, visible in the view
    * @param lineNumber the line where the pattern is located in the file
    * @param columnNumber the column where the pattern is located in the file
    */
    // FIXME Now disabled/remove because not really needed here. While coding XmlTypeParser I noticed that
    //       this func should return bool if successful or not. But don't wanted update all other stuff...
    // virtual void addNode(const int nodeType, const QString &text, const int lineNumber, const int columnNumber = 0) = 0;

    /**
    * Call this function at the beginning of addNode() to test if you must go on.
    * @note This function ensures that a "not enabled" option is treated as "not checked"
    *       and that @c p_usefulOptions is filled.
    * @param nodeType the type of the new node, like header or paragraph
    * @return false when @p nodeType is not wanted to show.
    */
    bool nodeTypeIsWanted(int nodeType);

    /**
    * Call this function at the end of addNode() to set the data of the new node.
    * This function ensures not only to expand the node dependent on @c p_viewExpanded
    * but also independent of @c p_viewExpanded that the probably current node will expand.
    * @param node the new added node
    * @param nodeType the type of the new node, like header or paragraph
    * @param text the caption of the new node, visible in the view
    * @param lineNumber the line where the pattern is located in the file
    */
    void setNodeProperties(QTreeWidgetItem *const node, const int nodeType, const QString &text, const int lineNumber, const int columnNumber = 0);

    /**
     * @return the last added node, which is p_lastNode
     */
    QTreeWidgetItem *lastNode() const { return p_lastNode; }

    /**
     * Use this set of flags to indicate a changed state in some function.
     * Typically you should add the macro @c Me_At_Work to avaoid conflicts with
     * other functions/flags. This set is cleared at the begin of parse().
     */
    QSet<QString>                   m_funcAtWork;

    /**
     * This is the main variable where all functions should work on.
     * Starting from appendNextLine() until lastly parseDocument() can do its
     * job.
     */
    QString                         m_line;

    /**
     * Add to this set of flags @c NodeType which will always added to its top
     * level node. So, these will never appear nested in the tree.
     * Hint: It's currently only used for FIXME/TODO nodes
     */
    QSet<int>                       m_detachedNodeTypes;

    /**
     * Measure the gone parse time. Started in parse() and checked in appendNextLine()
     * to call qApp->processEvents() to keep the editing responsive.
     */
    QElapsedTimer                   m_runTime;

private:
    KTextEditor::Document          *p_document = nullptr;
    QTreeWidget                    *p_indexTree = nullptr;
    IndexView                      *p_view = nullptr;
    QMenu                           p_menu;
    QAction                        *p_viewTree = nullptr;
    QAction                        *p_addIcons = nullptr;
    QAction                        *p_viewExpanded = nullptr;

    struct NodeTypeStruct {
        NodeTypeStruct() {};
        NodeTypeStruct(const  QString &name, QIcon icon, QAction *option = nullptr)
        : name(name), icon(icon), option(option) {};
        QString     name;
        QIcon       icon;
        QAction    *option = nullptr;
    };
    QHash<int, NodeTypeStruct>      p_nodeTypes;

    QHash<int, QTreeWidgetItem*>    p_rootNodes;

    struct DependencyPair {
        DependencyPair(QAction *t ,QAction *y) : dDent(t), dDency(y) {};
        QAction* dDent;  // dependent
        QAction* dDency; // dependency
    };
    QList<DependencyPair>           p_modifierOptions;

    QSet<QAction*>                  p_usefulOptions;
    int                             p_lineNumber;           // Counter in appendNextLine()

    // No one should change this variable except Parser::parse and Parser::setNodeProperties
    QTreeWidgetItem                *p_lastNode = nullptr;

};

/**
 * The @c DummyParser class is a placeholder to keep things sane.
 *
 * @author loh.tar
 */

class DummyParser : public Parser
{
    Q_OBJECT

public:
    DummyParser(const QString &type, IndexView *view);
   ~DummyParser();

protected:
    enum NodeType {
        InfoNode
    };

    QString version() override { return QStringLiteral("1.0"); } ;
    QString author() override { return QStringLiteral("2018 loh.tar"); } ;

    void parseDocument() override;
    void addNode(const int nodeType, const QString &text);

    QString     m_docType;

};

#endif

// kate: space-indent on; indent-width 4; replace-tabs on;
