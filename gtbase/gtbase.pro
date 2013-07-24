TEMPLATE = lib
TARGET = gtbase
CONFIG += qt debug
HEADERS += gtcommon.h gtobject.h gtabstractdocument.h gtdocument.h \
    gtdocument_p.h gtdocpage.h gtdocpage_p.h gtdocmodel.h \
    gtdocloader.h gtdocpoint.h gtdocrange.h gtdocoutline.h \
    gtdocnote.h gtdocnote_p.h gtdocnotes.h
SOURCES += gtobject.cpp gtabstractdocument.cpp gtdocument.cpp \
    gtdocpage.cpp gtdocmodel.cpp gtdocloader.cpp gtdocpoint.cpp \
    gtdocrange.cpp gtdocoutline.cpp gtdocnote.cpp gtdocnotes.cpp

CONFIG(debug, debug|release) {
    DESTDIR = ../build/debug
} else {
    DESTDIR = ../build/release
}
