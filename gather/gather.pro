TEMPLATE = app
TARGET = gather
CONFIG += qt debug
QT += sql widgets network
HEADERS += gtapplication.h gtdocmanager.h gtmainwindow.h \
    gttabview.h gthometabview.h gtdoctabview.h gtmainsettings.h \
    gtlogindialog.h
SOURCES += gtmain.cpp gtapplication.cpp gtdocmanager.cpp \
    gtmainwindow.cpp gttabview.cpp gthometabview.cpp gtdoctabview.cpp \
    gtmainsettings.cpp gtlogindialog.cpp
FORMS += gtmainwindow.ui gthometabview.ui gtlogindialog.ui
RESOURCES = gather.qrc
INCLUDEPATH += $$PWD/../gtbase/gtbase
INCLUDEPATH += $$PWD/../gtview
INCLUDEPATH += $$PWD/../gtsvce/gtsvce

CONFIG(debug, debug|release) {
    DESTDIR = ../build/debug
} else {
    DESTDIR = ../build/release
}

unix: LIBS += -L$$DESTDIR -lgtbase -lgtview -lgtsvce
