/*   This file is part of KatePlugin-IndexView
 *
 *   FortranParser Class
 *   Copyright (C) 2018 loh.tar@googlemail.com
 *
 *   Inspired by fortran_parser.cpp, part of Kate's SymbolViewer
 *   Copyright (C) 2005 Roberto Quitiliani <roby(dot)q(AT)tiscali(dot)it>
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


#ifndef INDEXVIEW_FORTRANPARSER_CLASS_H
#define INDEXVIEW_FORTRANPARSER_CLASS_H

#include "program_parser.h"

/**
 * The \p FortranParser provide support for Fortran files
 *
 * @author loh.tar
 */
class FortranParser : public ProgramParser
{
    Q_OBJECT

public:
    FortranParser(IndexView *view);
   ~FortranParser();

protected:
   enum NodeType {
        ModuleNode = FirstNodeType,
        SubroutineNode,
        FunctionNode,
        TypeNode,
    };

    QString version() override { return QStringLiteral("0.6, Okt 2018"); } ;
    QString author() override { return QStringLiteral("2018 loh.tar \n\nInspired by \n2005 Roberto Quitiliani"); } ;

    void parseDocument() override;
    bool lineIsGood() override { return true; };
    void removeStrings() override;
    void removeComment() override;

    void beginBlock(const QString  &type);
    void endBlock(const QString  &type);

    QStack<QString> m_BlockStack;

    QRegExp      m_rxModule;
    QRegExp      m_rxSubroutine;
    QRegExp      m_rxType;
    QRegExp      m_rxFunction;
    QRegExp      m_rxEnd;
    QRegExp      m_rxProgram;

};

#endif

// kate: space-indent on; indent-width 4; replace-tabs on;
