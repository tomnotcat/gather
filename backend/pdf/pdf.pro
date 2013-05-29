TEMPLATE = lib
TARGET = pdf
CONFIG += qt
HEADERS += pdfdocument.h pdfdocpage.h
SOURCES += pdfdocument.cpp pdfdocpage.cpp
INCLUDEPATH += ../../gtbase

CONFIG(debug, debug|release) {
    DESTDIR = ../../build/debug
} else {
    DESTDIR = ../../build/release
}
