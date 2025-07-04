/*
 * Copyright (C) 2011 ~ 2018 Deepin Technology Co., Ltd.
 *
 * Author:     listenerri <listenerri@gmail.com>
 *
 * Maintainer: listenerri <listenerri@gmail.com>
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

#include "fullfilesystemframe.h"
#include "qtranslator.h"
#include <QApplication>

#include <DLog>
#include <DApplication>

DCORE_USE_NAMESPACE
DWIDGET_USE_NAMESPACE

int main(int argc, char *argv[])
{
    DApplication app(argc, argv);
    app.setOrganizationName("deepin");
    app.setApplicationName("dde-full-filesystem");
    //app.setApplicationVersion("2015.1.0");

    DLogManager::registerConsoleAppender();
    DLogManager::registerFileAppender();

    QTranslator translator;
    translator.load("/usr/share/gxde-session-ui/translations/gxde-session-ui_" + QLocale::system().name());
    app.installTranslator(&translator);

    FullFilesystemFrame frame;
    frame.show();

    return app.exec();
}
