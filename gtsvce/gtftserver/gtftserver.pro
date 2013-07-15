TEMPLATE = app
TARGET = gtftserver
CONFIG += qt debug
QT -= gui
QT += network
HEADERS +=
SOURCES += gtftserver.cpp
INCLUDEPATH += $$PWD/../../gtbase
INCLUDEPATH += $$PWD/../gtsvce
INCLUDEPATH += $$PWD/../../

CONFIG(debug, debug|release) {
    DESTDIR = ../../build/debug
} else {
    DESTDIR = ../../build/release
}

unix: LIBS += -L$$DESTDIR -lgtbase -lgtsvce
