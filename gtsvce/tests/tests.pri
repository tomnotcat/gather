CONFIG += debug
QT += network
INCLUDEPATH += ../../message
INCLUDEPATH += ../../gtsvce
INCLUDEPATH += ../../../gtbase/gtbase

CONFIG(debug, debug|release) {
    DESTDIR = ../../../build/debug
} else {
    DESTDIR = ../../../build/release
}

unix: LIBS += -L$$DESTDIR -lgtbase -lgtsvce -lprotobuf
