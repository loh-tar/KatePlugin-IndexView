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

// TODO Get rid of these enum and related stuff
enum IconType {
    BlueIcon,
    Head1Icon = BlueIcon,
    ClassIcon = BlueIcon,

    GreenYellowIcon,
    Head2Icon = GreenYellowIcon,

    RedBlueIcon,
    Head3Icon = RedBlueIcon,
    FunctionIcon = RedBlueIcon,
    MethodIcon = RedBlueIcon,

    GreyIcon,
    Head4Icon = GreyIcon,
    NamespaceIcon = GreyIcon,

    SmallYellowIcon,
    Head5Icon = SmallYellowIcon,

    SmallRedIcon,
    VariableIcon = SmallRedIcon,

    SmallGreenIcon,

    SmallDarkBlueIcon,
    ConstantIcon = SmallDarkBlueIcon,

    SmallBlueIcon,
    Head6Icon = SmallBlueIcon,

    SmallGreyIcon,
    ParaIcon = SmallGreyIcon,
};

QIcon getIcon(IconType type);

/**
 * This little function return an on the fly generated icon.
 * When called only with a size argument is a color auto chosen from an intern list
 * of Qt::GlobalColor. With each such auto gen call is an other color used. When no
 * new color is available the frst known color is used again.
 * @parm size How many circle to use 1-3 or -1 to reset auto color counter
 * @parm qtGlobalColorEnum A color from Qt::GlobalColor or -1 for auto color
 */
QIcon getIcon(const int size, const int qtGlobalColorEnum = -1);

}

#endif

// kate: space-indent on; indent-width 4; replace-tabs on;
