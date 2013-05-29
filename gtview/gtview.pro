TEMPLATE = lib
CONFIG += qt
QT += widgets
HEADERS += gtdocview.h
SOURCES += gtdocview.cpp
INCLUDEPATH += ../gtbase

CONFIG(debug, debug|release) {
    DESTDIR = ../build/debug
} else {
    DESTDIR = ../build/release
}
