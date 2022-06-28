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

// FIXME Add an originator or source of these icons

// I have not replaced these construct of hard coded bitmaps by the typical resource
// approach. Anyone knows why that was not done?

#include <QIcon>
#include <QPainter>
#include <QDebug>

#include "icon_collection.h"

// TODO Add more icon colors. One idea is to keep only one template per
// icon type and change the colors in the header of each template on the fly.
// NOTE "icon type" means here NOT the enum IconType! When you look that the
// arrays, you may notice that there are only three types of icons but with different
// colors. Without the colors "None" and "#000000", count the icons 3, 8 or 12 colors.
// So we could keep only three arrays and fill-in the needed colors by some function
// which should adding new colored icons more easily.

namespace IconCollection {

QIcon getIcon(const int size, const int colorIdx/* = -1 */)
{
    static int autoColor = 0;
    const static QList<int> autoColors = { Qt::blue, Qt::red, Qt::green, Qt::cyan, Qt::magenta, Qt::yellow, Qt::gray };

    const qreal pixmapSize = 24.0;
    const qreal pixmapHalf = pixmapSize / 2.0;
    const qreal penWidth = 1.0;
    const qreal circleSize = pixmapSize / 4.0 - penWidth;
    const qreal circleHalf = circleSize / 2.0;

    QPixmap pixmap(pixmapSize,pixmapSize);
    pixmap.fill();

    if (size < 1) {
        // No error, just reset our auto color counter
        autoColor = 0;
        return QIcon(pixmap);
    }

    QColor color;
    if (colorIdx < 0) {
        color = QColor((Qt::GlobalColor)autoColors.at(autoColor++));
        if (autoColors.size() == autoColor) {
            autoColor = 0;
        }
    }

    QPainter painter(&pixmap);
    QPen pen = painter.pen();
    pen.setWidthF(penWidth);
    painter.setPen(pen);

    QBrush brush(color);

    if (size == 3) {
        brush.setColor(color.darker(150));
        painter.setBrush(brush);
        painter.drawEllipse(QPointF(pixmapHalf - circleSize, pixmapHalf - circleSize), circleSize * 1.2, circleSize * 1.2);

        brush.setColor(color);
        painter.setBrush(brush);
        painter.drawEllipse(QPointF(pixmapHalf + circleSize, pixmapHalf), circleSize * 1.2, circleSize * 1.2);

        brush.setColor(color.lighter(150));
        painter.setBrush(brush);
        painter.drawEllipse(QPointF(pixmapHalf - circleSize, pixmapHalf + circleSize), circleSize, circleSize);

    } else if (size == 2) {
        brush.setColor(color.lighter(150));
        painter.setBrush(brush);
        painter.drawEllipse(QPointF(pixmapHalf - circleSize, pixmapHalf - circleSize), circleSize * 1.2, circleSize * 1.2);

        brush.setColor(color);
        painter.setBrush(brush);
        painter.drawEllipse(QPointF(pixmapHalf + circleSize, pixmapHalf + circleSize), circleSize * 1.2, circleSize * 1.2);

    } else {
        painter.setBrush(brush);
        painter.drawEllipse(QPointF(pixmapHalf/* + circleSize*/, pixmapHalf), circleSize * 1.2, circleSize * 1.2);
    }

    return QIcon(pixmap);
}

static const char* const blueXpm[] = {
    "16 16 10 1",
    " 	c None",
    ".	c #000000",
    "+	c #A4E8FC",
    "@	c #24D0FC",
    "#	c #001CD0",
    "$	c #0080E8",
    "%	c #C0FFFF",
    "&	c #00FFFF",
    "*	c #008080",
    "=	c #00C0C0",
    "     ..         ",
    "    .++..       ",
    "   .+++@@.      ",
    "  .@@@@@#...    ",
    "  .$$@@##.%%..  ",
    "  .$$$##.%%%&&. ",
    "  .$$$#.&&&&&*. ",
    "   ...#.==&&**. ",
    "   .++..===***. ",
    "  .+++@@.==**.  ",
    " .@@@@@#..=*.   ",
    " .$$@@##. ..    ",
    " .$$$###.       ",
    " .$$$##.        ",
    "  ..$#.         ",
    "    ..          "
};


static const char* const greyXpm[] = {
    "16 16 10 1",
    " 	c None",
    ".	c #000000",
    "+	c #B8B8B8",
    "@	c #8A8A8A",
    "#	c #212121",
    "$	c #575757",
    "%	c #CCCCCC",
    "&	c #9A9A9A",
    "*	c #4D4D4D",
    "=	c #747474",
    "     ..         ",
    "    .++..       ",
    "   .+++@@.      ",
    "  .@@@@@#...    ",
    "  .$$@@##.%%..  ",
    "  .$$$##.%%%&&. ",
    "  .$$$#.&&&&&*. ",
    "   ...#.==&&**. ",
    "   .++..===***. ",
    "  .+++@@.==**.  ",
    " .@@@@@#..=*.   ",
    " .$$@@##. ..    ",
    " .$$$###.       ",
    " .$$$##.        ",
    "  ..$#.         ",
    "    ..          "
};


static const char* const greenYellowXpm[] = {
    "16 16 14 1",
    " 	c None",
    ".	c #000000",
    "+	c #C0FFC0",
    "@	c #00FF00",
    "#	c #008000",
    "$	c #00C000",
    "%	c #C0FFFF",
    "&	c #00FFFF",
    "*	c #008080",
    "=	c #00C0C0",
    "-	c #FFFFC0",
    ";	c #FFFF00",
    ">	c #808000",
    ",	c #C0C000",
    "     ..         ",
    "    .++..       ",
    "   .+++@@.      ",
    "  .@@@@@#...    ",
    "  .$$@@##.%%..  ",
    "  .$$$##.%%%&&. ",
    "  .$$$#.&&&&&*. ",
    "   ...#.==&&**. ",
    "   .--..===***. ",
    "  .---;;.==**.  ",
    " .;;;;;>..=*.   ",
    " .,,;;>>. ..    ",
    " .,,,>>>.       ",
    " .,,,>>.        ",
    "  ..,>.         ",
    "    ..          "
};


static const char* const redBlueXpm[] = {
    "16 16 14 1",
    " 	c None",
    ".	c #000000",
    "+	c #FF7FE5",
    "@	c #FF00C7",
    "#	c #7F0066",
    "$	c #BC0096",
    "%	c #C0FFFF",
    "&	c #00FFFF",
    "*	c #008080",
    "=	c #00C0C0",
    "-	c #D493FF",
    ";	c #A100FF",
    ">	c #470082",
    ",	c #6B00B7",
    "     ..         ",
    "    .++..       ",
    "   .+++@@.      ",
    "  .@@@@@#...    ",
    "  .$$@@##.%%..  ",
    "  .$$$##.%%%&&. ",
    "  .$$$#.&&&&&*. ",
    "   ...#.==&&**. ",
    "   .--..===***. ",
    "  .---;;.==**.  ",
    " .;;;;;>..=*.   ",
    " .,,;;>>. ..    ",
    " .,,,>>>.       ",
    " .,,,>>.        ",
    "  ..,>.         ",
    "    ..          "
};


static const char* const smallYellowXpm[] = { //original
    "16 16 5 1",
    "       c None",
    ".      c #000000",
    "+      c #FCFC80",
    "@      c #E0BC38",
    "#      c #F0DC5C",
    "                ",
    "                ",
    "                ",
    "          ..    ",
    "         .++..  ",
    "        .+++++. ",
    "       .+++++@. ",
    "    .. .##++@@. ",
    "   .++..###@@@. ",
    "  .+++++.##@@.  ",
    " .+++++@..#@.   ",
    " .##++@@. ..    ",
    " .###@@@.       ",
    " .###@@.        ",
    "  ..#@.         ",
    "    ..          "
};


static const char* const smallRedXpm[] = { //redBlueXpm
    "16 16 5 1",
    "       c None",
    ".      c #000000",
    "+      c #FF7FE5",
    "@      c #FF00C7",
    "#      c #BC0096",
    "                ",
    "                ",
    "                ",
    "          ..    ",
    "         .++..  ",
    "        .+++++. ",
    "       .+++++@. ",
    "    .. .##++@@. ",
    "   .++..###@@@. ",
    "  .+++++.##@@.  ",
    " .+++++@..#@.   ",
    " .##++@@. ..    ",
    " .###@@@.       ",
    " .###@@.        ",
    "  ..#@.         ",
    "    ..          "
};


static const char* const smallGreenXpm[] = { //greenYellowXpm
    "16 16 5 1",
    "       c None",
    ".      c #000000",
    "+      c #00FF00",
    "@      c #008000",
    "#      c #00C000",
    "                ",
    "                ",
    "                ",
    "          ..    ",
    "         .++..  ",
    "        .+++++. ",
    "       .+++++@. ",
    "    .. .##++@@. ",
    "   .++..###@@@. ",
    "  .+++++.##@@.  ",
    " .+++++@..#@.   ",
    " .##++@@. ..    ",
    " .###@@@.       ",
    " .###@@.        ",
    "  ..#@.         ",
    "    ..          "
};


static const char* const smallGreyXpm[] = { //greyXpm
    "16 16 5 1",
    "       c None",
    ".      c #000000",
    "+      c #9A9A9A",
    "@      c #4D4D4D",
    "#      c #747474",
    "                ",
    "                ",
    "                ",
    "          ..    ",
    "         .++..  ",
    "        .+++++. ",
    "       .+++++@. ",
    "    .. .##++@@. ",
    "   .++..###@@@. ",
    "  .+++++.##@@.  ",
    " .+++++@..#@.   ",
    " .##++@@. ..    ",
    " .###@@@.       ",
    " .###@@.        ",
    "  ..#@.         ",
    "    ..          "
};


static const char* const smallDarkBlueXpm[] = { //blueXpm
    "16 16 5 1",
    "       c None",
    ".      c #000000",
    "+      c #24D0FC",
    "@      c #001CD0",
    "#      c #0080E8",
    "                ",
    "                ",
    "                ",
    "          ..    ",
    "         .++..  ",
    "        .+++++. ",
    "       .+++++@. ",
    "    .. .##++@@. ",
    "   .++..###@@@. ",
    "  .+++++.##@@.  ",
    " .+++++@..#@.   ",
    " .##++@@. ..    ",
    " .###@@@.       ",
    " .###@@.        ",
    "  ..#@.         ",
    "    ..          "
};


static const char* const smallBlueXpm[] = { //blueXpm
    "16 16 5 1",
    "       c None",
    ".      c #000000",
    "+      c #00FFFF",
    "@      c #008080",
    "#      c #00C0C0",
    "                ",
    "                ",
    "                ",
    "          ..    ",
    "         .++..  ",
    "        .+++++. ",
    "       .+++++@. ",
    "    .. .##++@@. ",
    "   .++..###@@@. ",
    "  .+++++.##@@.  ",
    " .+++++@..#@.   ",
    " .##++@@. ..    ",
    " .###@@@.       ",
    " .###@@.        ",
    "  ..#@.         ",
    "    ..          "
};


QIcon getIcon(IconType type)
{
    switch (type) {
        case BlueIcon:          return QIcon(QPixmap(const_cast<const char**>(blueXpm))); break;
        case GreenYellowIcon:   return QIcon(QPixmap(const_cast<const char**>(greenYellowXpm))); break;
        case RedBlueIcon:       return QIcon(QPixmap(const_cast<const char**>(redBlueXpm))); break;
        case GreyIcon:          return QIcon(QPixmap(const_cast<const char**>(greyXpm))); break;
        case SmallYellowIcon:   return QIcon(QPixmap(const_cast<const char**>(smallYellowXpm))); break;
        case SmallRedIcon:      return QIcon(QPixmap(const_cast<const char**>(smallRedXpm))); break;
        case SmallGreenIcon :   return QIcon(QPixmap(const_cast<const char**>(smallGreenXpm))); break;
        case SmallDarkBlueIcon: return QIcon(QPixmap(const_cast<const char**>(smallDarkBlueXpm))); break;
        case SmallBlueIcon:     return QIcon(QPixmap(const_cast<const char**>(smallBlueXpm))); break;
        case SmallGreyIcon:     return QIcon(QPixmap(const_cast<const char**>(smallGreyXpm))); break;

        // Just to satisfy the compiler
        default: return QIcon(QPixmap(const_cast<const char**>(blueXpm))); break;
    }
}

}

// kate: space-indent on; indent-width 4; replace-tabs on;
