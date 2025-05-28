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
#include <QPointer>
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

enum NodeData {
    Line = Qt::UserRole, // Where the pattern is located
    Column,              // Where the pattern is located
    EndLine              // Line number for which the item is still relevant/responsible
};

class KatePluginIndexView;

/**
 * The @c Parser class must be the base for all @c IndexView parser.
 * However it may not recommended to use this class as a direct parent for new
 * written parser. There are (currently three) derivated master parser available
 * which offer a couple of helpful features.
 * @see DocumentParser, ProgramParser, XmlTypeParser
 *
 * The @c Parser class do a little more than only to parse the document, it is
 * also some kind of data container and care taker to avoid flicker.
 *
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
    friend class XmlTypeParser;

public:
    Parser(QObject *view, KTextEditor::Document *doc);
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
    static Parser *create(KTextEditor::Document *doc, const QString &type, QObject *view);

    /**
     * Load user configured individual preferences
     */
    void loadSettings();

    /**
     * Save user configured individual preferences
     */
    void saveSettings();

    /**
     * This function returns the document type the parser was created for.
     * @return document type
     */
    QString docType() { return p_docType; };

    KTextEditor::Document * document() { return p_document; };

    /**
     * This function is used to avoid parsing restart when a parsing is already running.
     * @return true when the parser is already parsing, otherwise false
     */
    bool isParsing() { return p_parsingIsRunning; };

    /**
     * This function is called by IndexView::docEdited() to hint the parser that
     * a parsing is needed. Without this call parse() does nothing!
     */
    void docNeedParsing() { p_docNeedParsing = true; };

    /**
     * This function must be used to protect for unneeded parsing, or we are doomed!
     * HINT: Introduced to never miss an update but also to avoid unneeded parsing after
     * a document switch.
     */
    bool needsUpdate() { return p_docNeedParsing; };

    /**
     * This function return the last parsed tree.
     * WARNING: Never burn (delete) this tree, Parser take care of him
     * @return the last updated index tree
     */
    QTreeWidget *indexTree() { return p_indexTree; };

    /**
     * This function return the old, outdated index tree which is no longer solid.
     * WARNING: Never burn (delete) this tree, burnDownMustyTree() take care of him
     * @return the old, outdated index tree
     */
    QTreeWidget *mustyTree() { return p_mustyTree; };

    /**
     * This function is pretty important! Must be called after all updates are done
     * and will not only remove old wood but also ensure the tree keep the old scroll
     * position.
     */
    void burnDownMustyTree();

    // TODO Are these two needed? The filter stuff works sometimes strange.
    // But atm I cant say how exact it should behave.
    bool isTreeFiltered() { return p_filtered; };
    void treeIsFiltered(bool filtred) { p_filtered = filtred; };

    /**
     * @return the index list
     */
    QList<QTreeWidgetItem *> *indexList() { return &p_indexList; };

    /**
    * This is the main access function to parse the document. These will call
    * prepareForParse() and parseDocument(). Before and after that will done
    * some janitor task like update the context menu to hide unneeded options.
    * NOTE: When prior was not called docNeedParsing() nothing is done.
    * Only some master classes may need to implemented an own
    * version to init some special variables or to do other special treatment.
    * Ensure in that case to call Parser::parse() at some point.
    */
    virtual void parse();

    /**
     * Get the context menu which hold all view settings.
     */
    QMenu *contextMenu() { return &p_menu; };

    /**
     * @return true when the view has to be sorted
     */
    bool showSorted() { return p_viewSort->isChecked(); }

    /**
     * @return true when the view has to be expanded
     */
    bool showExpanded() { return p_viewExpanded->isChecked(); }

    /**
     * @return true when the view has to be a tree, not a list
     */
    bool showAsTree() { return p_viewTree->isChecked(); }

Q_SIGNALS:
    /**
     * Since we parse in time slices, we need to inform the IndexView that we are
     * done.
     */
    void parsingDone(Parser *parser);

protected Q_SLOTS:
    void menuActionTriggered();

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
     * Call this function in an ctor of a (master) class to add nesting options to the context menu.
     * Use @p adjust, when needed. E.g. DocumentParser need that, XmlTypeParser do not. It is related
     * how/when the nesting is calculated, before or after calling setNodeProperties(..).
     * @param adjust to use @c p_nestingLevelAdjustment or not
     */
    void useNestingOptions(bool adjust = false);

    /**
     * Call this function e.g. in prepareForParse() to init @c p_nestingLevel and @c p_nestingAllowed
     */
    void resetNesting();

    /**
     * Register an icon to use for @p nodeType.
     * See @c IconCollection::getIcon for @p size and @p qtGlobalColorEnum.
     */
    void setNodeTypeIcon(const int nodeType, const int size, const int qtGlobalColorEnum);

    /**
     * Register an icon to use for @p nodeType.
     * See @c IconCollection::getIcon
     */
    void setNodeTypeIcon(const int nodeType, const IconCollection::IconType iconType);

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
    * You can call this function at the beginning of addNode() to test if you must go on.
    * @note This function ensures that a "not enabled" option is treated as "not checked"
    *       and that @c p_usefulOptions is filled. It's also called in setNodeProperties()
    * @param nodeType the type of the new node, like header or paragraph
    * @return false when @p nodeType is not wanted to show.
    */
    bool nodeTypeIsWanted(int nodeType);

    /**
    * Call this function at the end of addNode() to set the data of the new node.
    * This function ensures also to expand the nodes dependent on @c p_viewExpanded.
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
    /**
     * This function is only called by Parser::create
     */
    void finalizeSetup(const QString &docType);

    KTextEditor::Document          *p_document; // Our doc where we work on, once set in ctor
    QString                         p_docType;  // The type of p_document, once set in ctor
    bool                            p_parsingIsRunning = false;
    bool                            p_docNeedParsing = true;
    QTreeWidget                    *p_indexTree = nullptr;
    QPointer<QTreeWidget>           p_mustyTree;
    bool                            p_filtered = false;
    QList<QTreeWidgetItem *>        p_indexList;
    QMenu                           p_menu;
    QAction                        *p_viewSort = nullptr;
    QAction                        *p_viewTree = nullptr;
    QAction                        *p_addIcons = nullptr;
    QAction                        *p_viewExpanded = nullptr;
    bool                            p_viewOptionsChanged = false;

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

    QAction                        *p_nesting1 = nullptr;
    QAction                        *p_nesting2 = nullptr;
    QAction                        *p_nesting3 = nullptr;
    QAction                        *p_nesting4 = nullptr;
    int                             p_nestingAllowed =  100000;
    int                             p_nestingLevel = 0;           // You must count +/- with this the nesting
    int                             p_nestingLevelAdjustment = 0; // To adjust p_nestingLevel, managed by useNestingOptions()
    int                             p_maxNesting =  -1;

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
    DummyParser(QObject *view, KTextEditor::Document *doc);
   ~DummyParser();

protected:
    enum NodeType {
        InfoNode
    };

    QString version() override { return QStringLiteral("1.0"); } ;
    QString author() override { return QStringLiteral("2018 loh.tar"); } ;

    void parseDocument() override;
    void addNode(const int nodeType, const QString &text);

};

#endif

// kate: space-indent on; indent-width 4; replace-tabs on;
