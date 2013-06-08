TEMPLATE = lib
CONFIG += qt debug
QT += widgets
HEADERS += gtdocview.h gtdocrendercache.h
SOURCES += gtdocview.cpp gtdocrendercache.cpp
INCLUDEPATH += ../gtbase

CONFIG(debug, debug|release) {
    DESTDIR = ../build/debug
} else {
    DESTDIR = ../build/release
}
