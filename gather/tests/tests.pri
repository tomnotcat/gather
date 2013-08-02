CONFIG += debug
INCLUDEPATH += ../../gtapp
INCLUDEPATH += ../../../gtbase/gtbase

CONFIG(debug, debug|release) {
    DESTDIR = ../../../build/debug
} else {
    DESTDIR = ../../../build/release
}

unix: LIBS += -L$$DESTDIR -lgtapp -lgtbase -lgtview -lgtsvce
