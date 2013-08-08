include(../../gtbase/message/protobuf.pri)

TEMPLATE = lib
TARGET = gtsvce-message
QT -= gui
CONFIG += staticlib qt debug
PROTOS += gtusermessage.proto gtftmessage.proto

CONFIG(debug, debug|release) {
    DESTDIR = ../../build/debug
} else {
    DESTDIR = ../../build/release
}
