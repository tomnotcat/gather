TEMPLATE = app
TARGET = gtuserserver
CONFIG += qt debug
QT -= gui
QT += network
HEADERS +=
SOURCES += gtuserserver.cpp
INCLUDEPATH += $$PWD/../../gtbase/gtbase
INCLUDEPATH += $$PWD/../gtsvce
INCLUDEPATH += $$PWD/../../

CONFIG(debug, debug|release) {
    DESTDIR = ../../build/debug
} else {
    DESTDIR = ../../build/release
}

unix: LIBS += -L$$DESTDIR -lgtbase -lgtsvce
