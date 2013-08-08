include(protobuf.pri)

TEMPLATE = lib
TARGET = gtbase-message
QT -= gui
CONFIG += staticlib qt debug
PROTOS += gtdocmessage.proto

CONFIG(debug, debug|release) {
    DESTDIR = ../../build/debug
} else {
    DESTDIR = ../../build/release
}
