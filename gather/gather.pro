TEMPLATE = app
TARGET = gather
CONFIG += qt debug
QT += widgets network
HEADERS += gtapplication.h gtmainwindow.h gttabview.h \
    gthometabview.h gtdoctabview.h gtmainsettings.h gtlogindialog.h
SOURCES += gtmain.cpp gtapplication.cpp gtmainwindow.cpp \
    gttabview.cpp gthometabview.cpp gtdoctabview.cpp \
    gtmainsettings.cpp gtlogindialog.cpp
FORMS += gtmainwindow.ui gthometabview.ui gtlogindialog.ui
RESOURCES = gather.qrc
INCLUDEPATH += $$PWD/../gtbase
INCLUDEPATH += $$PWD/../gtview
INCLUDEPATH += $$PWD/../gtsvce/gtsvce

CONFIG(debug, debug|release) {
    DESTDIR = ../build/debug
} else {
    DESTDIR = ../build/release
}

unix: LIBS += -L$$DESTDIR -lgtbase -lgtview -lgtsvce
