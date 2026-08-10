/*
 * Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
 *
 * Author:     kirigaya <kirigaya@mkacg.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "icondata.h"
#include <QDebug>

QDebug operator<<(QDebug arg, const IconData &data)
{
    arg << "width = " << data.width << Qt::endl;
    arg << "height = " << data.height << Qt::endl;
    arg << "rowstride = " << data.rowstride << Qt::endl;
    arg << "alpha = " << data.alpha << Qt::endl;
    arg << "per = " << data.bit << Qt::endl;
    arg << "cancel = " << data.cannel << Qt::endl;
    arg << "array = " << data.array << Qt::endl;
    return arg;
}

QDBusArgument &operator<<(QDBusArgument &arg, const IconData &data)
{
    arg.beginStructure();
    arg << data.width << data.height << data.rowstride << data.alpha << data.bit << data.cannel << data.array;
    arg.endStructure();

    return arg;
}

const QDBusArgument &operator>>(const QDBusArgument &arg, IconData &data)
{
    arg.beginStructure();
    arg >> data.width >> data.height >> data.rowstride >> data.alpha >> data.bit >> data.cannel >> data.array;
    arg.endStructure();

    return arg;
}
