CONFIG += debug
INCLUDEPATH += ../../message
INCLUDEPATH += ../../gtbase

CONFIG(debug, debug|release) {
    DESTDIR = ../../../build/debug
} else {
    DESTDIR = ../../../build/release
}

unix: LIBS += -L$$DESTDIR -lgtbase -lprotobuf
