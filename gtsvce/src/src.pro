TEMPLATE = lib
TARGET = gtsvce
QT -= gui
QT += network
CONFIG += qt debug client server
INCLUDEPATH += ../../gtbase

CONFIG(client) {
    HEADERS += gtclient.h
    SOURCES += gtclient.cpp
}

CONFIG(server) {
    HEADERS += gtserver.h gtsession.h gtsessionmanager.h \
        gtusersession.h
    SOURCES += gtserver.cpp gtsession.cpp gtsessionmanager.cpp \
        gtusersession.cpp
}

CONFIG(debug, debug|release) {
    DESTDIR = ../../build/debug
} else {
    DESTDIR = ../../build/release
}
