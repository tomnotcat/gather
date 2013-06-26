TEMPLATE = lib
TARGET = gtsvce
CONFIG += qt debug
HEADERS += gtsession.h
SOURCES += gtsession.cpp
INCLUDEPATH += ../gtbase

CONFIG(debug, debug|release) {
    DESTDIR = ../build/debug
} else {
    DESTDIR = ../build/release
}
