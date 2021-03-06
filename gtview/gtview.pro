TEMPLATE = lib
CONFIG += qt debug
QT += widgets
HEADERS += gtdocview.h gtdoccommand.h gtdocrendercache.h \
    gttocmodel.h gttocdelegate.h gttocview.h
SOURCES += gtdocview.cpp gtdoccommand.cpp gtdocrendercache.cpp \
    gttocmodel.cpp gttocdelegate.cpp gttocview.cpp
INCLUDEPATH += ../gtbase/gtbase

CONFIG(debug, debug|release) {
    DESTDIR = ../build/debug
} else {
    DESTDIR = ../build/release
}
