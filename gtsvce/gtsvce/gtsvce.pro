TEMPLATE = lib
TARGET = gtsvce
QT -= gui
QT += network
CONFIG += qt debug client server
INCLUDEPATH += ../message ../../gtbase/gtbase
HEADERS += gtrecvbuffer.h gtsvcutil.h gtfttemp.h
SOURCES += gtrecvbuffer.cpp gtsvcutil.cpp gtfttemp.cpp

CONFIG(client) {
    HEADERS += gtuserclient.h gtftclient.h
    SOURCES += gtuserclient.cpp gtftclient.cpp
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

unix: LIBS += -L$$DESTDIR -lgtbase -lgtsvce-message -lprotobuf
