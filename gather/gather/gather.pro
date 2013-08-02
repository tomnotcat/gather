TEMPLATE = app
TARGET = gather
CONFIG += qt debug
QT += widgets
SOURCES += gather.cpp
INCLUDEPATH += ../gtapp
INCLUDEPATH += ../../gtbase/gtbase

CONFIG(debug, debug|release) {
    DESTDIR = ../../build/debug
} else {
    DESTDIR = ../../build/release
}

unix: LIBS += -L$$DESTDIR -lgtapp -lgtbase
