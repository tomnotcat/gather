include(protobuf.pri)

TEMPLATE = lib
TARGET = gtsvce
QT -= gui
QT += network
CONFIG += qt debug client server
INCLUDEPATH += ../../gtbase
PROTOS += gtmessage.proto

CONFIG(client) {
    HEADERS += gtclient.h
    SOURCES += gtclient.cpp
}

CONFIG(server) {
    HEADERS += gtserver.h gtserver_p.h gtsession.h gtsession_p.h \
        gtuserserver.h gtusersession.h gtrecvbuffer.h gtsvcutil.h
    SOURCES += gtserver.cpp gtsession.cpp gtuserserver.cpp \
        gtusersession.cpp gtrecvbuffer.cpp gtsvcutil.cpp
}

CONFIG(debug, debug|release) {
    DESTDIR = ../../build/debug
} else {
    DESTDIR = ../../build/release
}

unix: LIBS += -L$$DESTDIR -lgtbase -lprotobuf
