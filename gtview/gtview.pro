TEMPLATE = lib
CONFIG += qt debug
QT += widgets
HEADERS += gtdocview.h gtdocrendercache.h gttocmodel.h \
    gttocdelegate.h
SOURCES += gtdocview.cpp gtdocrendercache.cpp gttocmodel.cpp \
    gttocdelegate.cpp
INCLUDEPATH += ../gtbase/gtbase

CONFIG(debug, debug|release) {
    DESTDIR = ../build/debug
} else {
    DESTDIR = ../build/release
}
