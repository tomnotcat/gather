TEMPLATE = lib
TARGET = pdf
CONFIG += qt debug
HEADERS += pdfdocument.h pdfdocpage.h
SOURCES += pdfdocument.cpp pdfdocpage.cpp
INCLUDEPATH += ../../gtbase

CONFIG(debug, debug|release) {
    DESTDIR = ../../build/debug/loader
} else {
    DESTDIR = ../../build/release/loader
}

metafile.target = $$DESTDIR/pdf.json
metafile.depends = pdf.json
metafile.commands = cp pdf.json $${metafile.target}
QMAKE_EXTRA_TARGETS += metafile
POST_TARGETDEPS += $${metafile.target}
