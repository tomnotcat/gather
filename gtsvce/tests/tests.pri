CONFIG += debug
QT += network
INCLUDEPATH += ../../../gtbase ../../src

CONFIG(debug, debug|release) {
    DESTDIR = ../../../build/debug
} else {
    DESTDIR = ../../../build/release
}

unix: LIBS += -L$$DESTDIR -lgtbase -lgtsvce
