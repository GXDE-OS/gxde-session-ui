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

#include "networksecretdialog.h"
#include "qtranslator.h"

#include <QApplication>

#include <DLog>
#include <DApplication>
#include "sessiontype.h"
#include <QCommandLineOption>
#include <QCommandLineParser>

#include <cstdio>
#include <unistd.h>

DCORE_USE_NAMESPACE
DWIDGET_USE_NAMESPACE

// 本程序是一次性的密码输入工具，stdout 是向 secret agent 返回结果的数据通道。
// dtkwidget 的静态对象在进程退出阶段析构时会破坏堆（glibc 报
// "corrupted size vs. prev_size while consolidating"），导致密码已经写出后
// 进程仍以 SIGABRT (core dumped) 结束，父进程会误判为获取密码失败、WiFi
// 连接随即以 no-secrets 失败。结果写出后直接结束进程，跳过静态析构。
[[noreturn]] static void finish(int code)
{
    std::fflush(nullptr);
    ::_exit(code);
}

int main(int argc, char *argv[])
{
    // dxcb is a X11 only platform plugin.
    if (!SessionType::isWayland())
        DApplication::loadDXcbPlugin();

    // translator 必须比 app 更早构造，析构时才会晚于 app，避免
    // QCoreApplication 清理 translators 时访问已销毁的对象。
    QTranslator translator;
    DApplication app(argc, argv);
    app.setOrganizationName("deepin");
    app.setApplicationName("dnetwork-secret-dialog");
    app.setTheme("dlight");
    app.setAttribute(Qt::AA_UseHighDpiPixmaps);

    DLogManager::registerConsoleAppender();
    DLogManager::registerFileAppender();

    translator.load("/usr/share/gxde-session-ui/translations/gxde-session-ui_" + QLocale::system().name());
    app.installTranslator(&translator);

    QCommandLineParser parser;
    parser.addHelpOption();
    parser.addPositionalArgument("filepath",
                                 "the file of json format data.\n"
                                 "data will be read from STDIN if filepath is not specified.",
                                 "[filepath]");
    parser.process(app);
    const QStringList &posArguments = parser.positionalArguments();

    QJsonDocument jsonDoc;
    QFile file;
    if (posArguments.isEmpty()) {
        qDebug() << "read json data from STDIN";
        if (!file.open(stdin, QFile::ReadOnly)) {
            qDebug() << "read from STDIN failed";
            finish(-2);
        }
    } else {
        file.setFileName(posArguments.first());
        if (!file.open(QFile::ReadOnly)) {
            qDebug() << "file:" << file.fileName()<< "open failed";
            finish(-1);
        }
    }

    jsonDoc = QJsonDocument::fromJson(file.readAll());
    if (jsonDoc.isEmpty()) {
        qDebug() << "invalid json data.";
        finish(-3);
    }

    file.close();

    NetworkSecretDialog dialog(jsonDoc);
    dialog.show();

    const int ret = app.exec();
    finish(ret);
}
