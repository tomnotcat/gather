TEMPLATE = lib
QT -= gui
CONFIG += qt debug
INCLUDEPATH += ../message
HEADERS += gtcommon.h gtobject.h gtserialize.h gtabstractdocument.h \
    gtdocument.h gtdocument_p.h gtdocmeta.h gtdocpage.h gtdocpage_p.h \
    gtdocmodel.h gtdocloader.h gtdocpoint.h gtdocrange.h gtlinkdest.h \
    gtbookmark.h gtbookmarks.h gtdocnote.h gtdocnotes.h
SOURCES += gtobject.cpp gtabstractdocument.cpp gtdocument.cpp \
    gtdocmeta.cpp gtdocpage.cpp gtdocmodel.cpp gtdocloader.cpp \
    gtdocpoint.cpp gtdocrange.cpp gtlinkdest.cpp gtbookmark.cpp \
    gtbookmarks.cpp gtdocnote.cpp gtdocnotes.cpp

CONFIG(debug, debug|release) {
    DESTDIR = ../../build/debug
} else {
    DESTDIR = ../../build/release
}

unix: LIBS += -L$$DESTDIR -lgtbase-message -lprotobuf
