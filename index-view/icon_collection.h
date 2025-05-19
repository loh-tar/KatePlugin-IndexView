/*   This file is part of KatePlugin-IndexView
 *
 *   Icon Collection
 *   Copyright (C) 2018 loh.tar@googlemail.com
 *
 *   Based loosely on
 *   plugin_katesymbolviewer
 *   Copyright (C) 2014 Kåre Särs <kare.sars@iki.fi>
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


#ifndef ICON_COLLECTION_H
#define ICON_COLLECTION_H

#include <QIcon>

namespace IconCollection {

// Don't use enums with a color name as node icon, use some descriptive name.
// I hope this way we can have a more uniform look across different file types
// and perhaps in the future an easier change to some better solution with e.g.
// an option to let the user switch between sets of icons like normal/dark-mode.
enum IconType {
    // Keep the AutoColorNIcons on top of this enum
    AutoColorReset = -1,
    AutoColor = -1,
    AutoColor3Icon,
    AutoColor2Icon,
    AutoColor1Icon,

    // Three 3 Dot Icons
    Blue3Icon,
        Head1Icon = Blue3Icon,
        ClassIcon = Blue3Icon,
    Red3Icon,
        UsesIcon = Red3Icon, // Perl
    Green3Icon,
        DocumentRootIcon = Green3Icon,
        InterfaceIcon = Green3Icon, // PHP
        SubroutineIcon = Green3Icon, // Fortran
    Cyan3Icon,
        NamespaceIcon = Cyan3Icon,
    Magenta3Icon,
        Head4Icon = Magenta3Icon,
    Yellow3Icon,
    Gray3Icon,
        ThisPluginIcon = Gray3Icon,

    // Two 2 Dot Icons
    Blue2Icon,
        Head2Icon = Blue2Icon,
        TypeIcon = Blue2Icon, // Fortran
    Red2Icon,
        PragmaIcon = Red2Icon, // Perl
    Green2Icon,
        FuncDefIcon = Green2Icon,
        MethodIcon = Green2Icon, // Python, Ruby
    Cyan2Icon,
        FuncDecIcon = Cyan2Icon,
    Magenta2Icon,
        Head5Icon = Magenta2Icon,
        TypedefIcon = Magenta2Icon,
    Yellow2Icon,
    Gray2Icon,
        EnumIcon = Gray2Icon,

    // One 1 Dot Icons
    Blue1Icon,
        Head3Icon = Blue1Icon,
    Red1Icon,
    Green1Icon,
        MacroIcon = Green1Icon,
    Cyan1Icon,
        BeginIcon = Cyan1Icon,
    Magenta1Icon,
        Head6Icon = Magenta1Icon,
        VariableIcon = Magenta1Icon,
    Yellow1Icon,
        FixmeTodoIcon = Yellow1Icon,
    Gray1Icon,
        ConstantIcon = Gray1Icon,
        ParaIcon = Gray1Icon,
        CommentIcon = Gray1Icon,
};

QIcon getPluginIcon();
QIcon getIcon(IconType type);

/**
 * This little function return an on the fly generated icon.
 * When called only with a size argument is a color auto chosen from an intern list
 * of Qt::GlobalColor. With each such auto gen call is an other color used. When no
 * new color is available the frst known color is used again.
 * @parm size How many circle to use 1-3 or -1 to reset auto color counter
 * @parm qtGlobalColorEnum A color from Qt::GlobalColor or -1 for auto color
 * @parm scale A factor to adjust the dimension of the icon, only used for plugin icon
 */
QIcon getIcon(const int size = AutoColorReset, const int qtGlobalColorEnum = AutoColor, const qreal scale = 1.0);

}

#endif

// kate: space-indent on; indent-width 4; replace-tabs on;
