TEMPLATE = app
TARGET = gather
CONFIG += qt debug
QT += widgets
HEADERS += gtmainwindow.h
SOURCES += gtmainwindow.cpp gtmain.cpp
RESOURCES = gather.qrc
INCLUDEPATH += $$PWD/../gtbase
INCLUDEPATH += $$PWD/../gtview

CONFIG(debug, debug|release) {
    DESTDIR = ../build/debug
} else {
    DESTDIR = ../build/release
}

unix: LIBS += -L$$DESTDIR -lgtbase
