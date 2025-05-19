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

namespace IconCollection {

QIcon getIcon(const int size/* = -1 */, const int qtGlobalColorEnum/* = -1 */, const qreal scale/* = 1.0*/)
{
    static int autoColor = 0;
    const static QList<int> autoColors = { Qt::blue, Qt::red, Qt::green, Qt::cyan, Qt::magenta, Qt::yellow, Qt::gray };

    const qreal pixmapSize = 24.0 * scale;
    const qreal pixmapHalf = pixmapSize / 2.0;
    const qreal penWidth = 1.0;
    const qreal circleSize = pixmapSize / 4.0 - penWidth;
    const qreal bigCircleSize = (pixmapSize / 4.0) * 1.2 - penWidth;
    //const qreal circleHalf = circleSize / 2.0;

    QPixmap pixmap(pixmapSize,pixmapSize);
    pixmap.fill(Qt::transparent);

    if (size < 1) {
        // No error, just reset our auto color counter
        autoColor = 0;
        return QIcon(pixmap);
    }

    QColor color;
    if (qtGlobalColorEnum < 0) {
        color = QColor((Qt::GlobalColor)autoColors.at(autoColor++));
        if (autoColors.size() == autoColor) {
            autoColor = 0;
        }
    } else {
        color = QColor((Qt::GlobalColor)qtGlobalColorEnum);
    }

    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    QPen pen = painter.pen();
    pen.setWidthF(penWidth);
    painter.setPen(pen);
    QBrush brush(color);

    if (size == 3) {
        brush.setColor(color.darker(150));
        painter.setBrush(brush);
        painter.drawEllipse(QRectF(penWidth, penWidth, bigCircleSize * 2.0, bigCircleSize * 2.0));

        brush.setColor(color);
        painter.setBrush(brush);
        painter.drawEllipse(QPointF(pixmapSize - bigCircleSize - penWidth, pixmapHalf), bigCircleSize, bigCircleSize);

        brush.setColor(color.lighter(150));
        painter.setBrush(brush);
        painter.drawEllipse(QPointF(pixmapHalf - circleSize, pixmapHalf + circleSize), circleSize, circleSize);

    } else if (size == 2) {
        brush.setColor(color.lighter(150));
        painter.setBrush(brush);
        painter.drawEllipse(QPointF(pixmapHalf - circleSize, pixmapHalf - circleSize), bigCircleSize, bigCircleSize);

        brush.setColor(color);
        painter.setBrush(brush);
        painter.drawEllipse(QPointF(pixmapHalf + circleSize, pixmapHalf + circleSize), bigCircleSize, bigCircleSize);

    } else {
        painter.setBrush(brush);
        painter.drawEllipse(QPointF(pixmapHalf/* + circleSize*/, pixmapHalf), bigCircleSize, bigCircleSize);
    }

    return QIcon(pixmap);
}


QIcon getIcon(IconType type)
{
    switch (type) {
        case AutoColor:      return getIcon(AutoColorReset); break;
        case AutoColor3Icon: return getIcon(3, AutoColor); break;
        case AutoColor2Icon: return getIcon(2, AutoColor); break;
        case AutoColor1Icon: return getIcon(1, AutoColor); break;

        case Blue3Icon:      return getIcon(3, Qt::blue); break;
        case Red3Icon:       return getIcon(3, Qt::red); break;
        case Green3Icon:     return getIcon(3, Qt::green); break;
        case Cyan3Icon:      return getIcon(3, Qt::cyan); break;
        case Magenta3Icon:   return getIcon(3, Qt::magenta); break;
        case Yellow3Icon:    return getIcon(3, Qt::yellow); break;
        case Gray3Icon:      return getIcon(3, Qt::gray); break;

        case Blue2Icon:      return getIcon(2, Qt::blue); break;
        case Red2Icon:       return getIcon(2, Qt::red); break;
        case Green2Icon:     return getIcon(2, Qt::green); break;
        case Cyan2Icon:      return getIcon(2, Qt::cyan); break;
        case Magenta2Icon:   return getIcon(2, Qt::magenta); break;
        case Yellow2Icon:    return getIcon(2, Qt::yellow); break;
        case Gray2Icon:      return getIcon(2, Qt::gray); break;

        case Blue1Icon:      return getIcon(1, Qt::blue); break;
        case Red1Icon:       return getIcon(1, Qt::red); break;
        case Green1Icon:     return getIcon(1, Qt::green); break;
        case Cyan1Icon:      return getIcon(1, Qt::cyan); break;
        case Magenta1Icon:   return getIcon(1, Qt::magenta); break;
        case Yellow1Icon:    return getIcon(1, Qt::yellow); break;
        case Gray1Icon:      return getIcon(1, Qt::gray); break;

        // Just to satisfy the compiler
        default:
            qDebug() << "FATAL getIcon not in case handled" << type;
            return getIcon(3, Qt::blue); break;
    }
}


QIcon getPluginIcon()
{
    // Looks a little strange, but this way we could make the icon configurable in the future more easily
    const qreal scale = 3.0; // In /usr/share/icons/breeze exist @2 and @3 variants see https://doc.qt.io/qt-6/qicon.html#high-dpi-icons
                             // FIXME Is there a way to check which scale factor may best? Or what size has the user set in Kate?
    switch (ThisPluginIcon) {
        case Blue3Icon:      return getIcon(3, Qt::blue, scale); break;
        case Red3Icon:       return getIcon(3, Qt::red, scale); break;
        case Green3Icon:     return getIcon(3, Qt::green, scale); break;
        case Cyan3Icon:      return getIcon(3, Qt::cyan, scale); break;
        case Magenta3Icon:   return getIcon(3, Qt::magenta, scale); break;
        case Yellow3Icon:    return getIcon(3, Qt::yellow, scale); break;
        case Gray3Icon:      return getIcon(3, Qt::gray, scale); break;

        // Just to satisfy the compiler
        default:
            qDebug() << "FATAL getPluginIcon not in case handled" << ThisPluginIcon;
            return getIcon(3, Qt::blue); break;
    }
}

}

// kate: space-indent on; indent-width 4; replace-tabs on;
