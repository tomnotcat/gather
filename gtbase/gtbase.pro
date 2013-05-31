TEMPLATE = lib
TARGET = gtbase
CONFIG += qt debug
HEADERS += gtcommon.h gtdocument.h gtdocument_p.h \
    gtdocpage.h gtdocpage_p.h gtdocloader.h
SOURCES += gtdocument.cpp gtdocpage.cpp gtdocloader.cpp

CONFIG(debug, debug|release) {
    DESTDIR = ../build/debug
} else {
    DESTDIR = ../build/release
}
