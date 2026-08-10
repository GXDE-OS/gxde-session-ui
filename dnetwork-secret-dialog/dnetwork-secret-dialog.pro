QT       += core gui widgets

TARGET = dnetwork-secret-dialog
TEMPLATE = app

DEFINES += QT_DEPRECATED_WARNINGS

CONFIG += c++11 link_pkgconfig
PKGCONFIG += dtkwidget x11

INCLUDEPATH += $$PWD/../global_util

SOURCES += \
        main.cpp \
    networksecretdialog.cpp \
    ../global_util/sessiontype.cpp

HEADERS += \
    networksecretdialog.h \
    ../global_util/sessiontype.h

RESOURCES += \
    resources.qrc

target.path = /usr/lib/deepin-daemon/
INSTALLS   += target
