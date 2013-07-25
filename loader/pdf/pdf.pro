TEMPLATE = lib
TARGET = pdf
CONFIG += qt debug
HEADERS += pdfdocument.h pdfpage.h pdfoutline.h
SOURCES += pdfdocument.cpp pdfpage.cpp pdfoutline.cpp
INCLUDEPATH += ../../gtbase/gtbase
INCLUDEPATH += ../../../mupdf/fitz
INCLUDEPATH += ../../../mupdf/pdf

CONFIG(debug, debug|release) {
    DESTDIR = ../../build/debug/loader
    LIBS += ../../../mupdf/build/debug/libfitz.a
    LIBS += ../../../mupdf/build/debug/libfreetype.a
    LIBS += ../../../mupdf/build/debug/libjbig2dec.a
    LIBS += ../../../mupdf/build/debug/libjpeg.a
    LIBS += ../../../mupdf/build/debug/libopenjpeg.a
    LIBS += ../../../mupdf/build/debug/libz.a
} else {
    DESTDIR = ../../build/release/loader
    LIBS += ../../../mupdf/build/release/libfitz.a
    LIBS += ../../../mupdf/build/release/libfreetype.a
    LIBS += ../../../mupdf/build/release/libjbig2dec.a
    LIBS += ../../../mupdf/build/release/libjpeg.a
    LIBS += ../../../mupdf/build/release/libopenjpeg.a
    LIBS += ../../../mupdf/build/release/libz.a
}

metafile.target = $$DESTDIR/pdf.json
metafile.depends = pdf.json
metafile.commands = cp pdf.json $${metafile.target}
QMAKE_EXTRA_TARGETS += metafile
POST_TARGETDEPS += $${metafile.target}
