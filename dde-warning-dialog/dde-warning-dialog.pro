#-------------------------------------------------
#
# Project created by QtCreator 2017-05-10T09:51:56
#
#-------------------------------------------------

include(../common.pri)

QT       += core gui widgets dbus

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = dde-warning-dialog
TEMPLATE = app

CONFIG += c++11 link_pkgconfig
PKGCONFIG += dtkwidget x11

INCLUDEPATH += $$PWD/../global_util

SOURCES += main.cpp \
    warningdialog.cpp \
    ../global_util/sessiontype.cpp

HEADERS  += \
    warningdialog.h \
    ../global_util/sessiontype.h

service.files += com.deepin.dde.WarningDialog.service
service.path = /usr/share/dbus-1/services/

target.path = /usr/lib/deepin-daemon/
INSTALLS   += target service
