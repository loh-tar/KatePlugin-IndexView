/*   This file is part of KatePlugin-IndexView
 *
 *   DocumentParser Class
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


#ifndef INDEXVIEW_DOCUMENTPARSER_CLASS_H
#define INDEXVIEW_DOCUMENTPARSER_CLASS_H

#include "parser.h"

class IndexView;
class KatePluginIndexView;

/**
 * The \p DocumentParser master class offer features for human readable files.
 *
 * @author loh.tar
 */
class DocumentParser : public Parser
{

public:
    DocumentParser(IndexView *view);
   ~DocumentParser();

   virtual void parse() override;

protected:

    /**
    * Overwrite these enum with an enhanced version in a sub class but ensure
    * to list RootNode first.
    * @see Parser::NodeType for more hints
    */
    enum NodeType {
        RootNode
    };

    /**
    * Overwrite these enum with an enhanced version in a sub class to keep
    * the code good readable but ensure to list EmptyLine first.
    */
    enum LineType {
        EmptyLine
    };

    /**
    * This function iterate with each call over the document and set @c m_line to
    * the next non empty line. Whereby some lines are treat as empty, like lines
    * with only a single char and lines matching @c m_rxIgnoreLine.
    */
    virtual bool nextLine();

    /**
    * Add a new node to the index view with respect of current view settings.
    * @param nodeType the type of the new node, like header or paragraph
    * @param text the caption of the new node, visible in the view
    * @param lineNumber the line where the pattern is located in the file
    */
    virtual void addNode(const int nodeType, const QString &text, const int lineNumber) override;

    /**
    * This function is called twice by parse() to ensure that not only all useful
    * options are visible but also that nothing is undesired skipped by parse().
    * @return a checksum which depend on visible view options
    */
    virtual QString disableDependentOptions();

    virtual void prepareForParse() override;

    /**
    * Initialize the history of already read lines, which effective fill the
    * history with empty lines. This function is typically called at the begin of
    * parsing with a suitable @p size but you may also call it depended on the
    * current parsend line in which case you can omit @p size to keep the buffer
    * size unchanged.
    * @param size is the number of lines to keep in the history
    */
    void initHistory(int size = -1);

    /**
    * Add a new line to the history and kept the history limited to the size
    * given in initHistory().
    * @param lineType the type of the new line to add, should be enum LineType
    * @param line the new line to add as your needs
    * @see initHistory()
    */
    void addToHistory(int lineType, const QString &line);

    int                      m_paraLineNumber;    // Where the current paragraph begins
    QQueue<int>              m_lineTypeHistory;
    QQueue<QString>          m_lineHistory;
    QRegExp                  m_rxIgnoreLine = QRegExp(QStringLiteral("^[-#*./]{1,2}$"));

private:
    int                      p_historySize;

};

#endif

// kate: space-indent on; indent-width 4; replace-tabs on;
