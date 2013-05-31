TEMPLATE = lib
CONFIG += qt debug
QT += widgets
HEADERS += gtdocview.h
SOURCES += gtdocview.cpp
INCLUDEPATH += ../gtbase

CONFIG(debug, debug|release) {
    DESTDIR = ../build/debug
} else {
    DESTDIR = ../build/release
}
