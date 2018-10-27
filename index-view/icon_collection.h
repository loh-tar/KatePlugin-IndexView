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

}

#endif

// kate: space-indent on; indent-width 4; replace-tabs on;
