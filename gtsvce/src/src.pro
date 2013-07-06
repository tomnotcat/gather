include(protobuf.pri)

TEMPLATE = lib
TARGET = gtsvce
QT -= gui
QT += network
CONFIG += qt debug client server
INCLUDEPATH += ../../gtbase
PROTOS += gtusermessage.proto gtftmessage.proto
HEADERS += gtrecvbuffer.h gtsvcutil.h
SOURCES += gtrecvbuffer.cpp gtsvcutil.cpp

CONFIG(client) {
    HEADERS += gtuserclient.h gtftupload.h gtftdownload.h
    SOURCES += gtuserclient.cpp gtftupload.cpp gtftdownload.cpp
}

CONFIG(server) {
    HEADERS += gtserver.h gtserver_p.h gtsession.h gtsession_p.h \
        gtuserserver.h gtusersession.h gtftserver.h gtftsession.h
    SOURCES += gtserver.cpp gtsession.cpp gtuserserver.cpp \
        gtusersession.cpp gtftserver.cpp gtftsession.cpp
}

CONFIG(debug, debug|release) {
    DESTDIR = ../../build/debug
} else {
    DESTDIR = ../../build/release
}

unix: LIBS += -L$$DESTDIR -lgtbase -lprotobuf
