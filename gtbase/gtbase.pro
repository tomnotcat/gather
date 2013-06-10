TEMPLATE = lib
TARGET = gtbase
CONFIG += qt debug
HEADERS += gtcommon.h gtabstractdocument.h gtdocument.h \
    gtdocument_p.h gtdocpage.h gtdocpage_p.h gtdocmodel.h gtdocloader.h
SOURCES += gtabstractdocument.cpp gtdocument.cpp gtdocpage.cpp \
    gtdocmodel.cpp gtdocloader.cpp

CONFIG(debug, debug|release) {
    DESTDIR = ../build/debug
} else {
    DESTDIR = ../build/release
}
