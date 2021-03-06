TEMPLATE = lib
CONFIG += qt debug
QT += sql widgets network
HEADERS += gtapplication.h gtdocmanager.h gtusermanager.h \
    gtusermanager_p.h gtmainwindow.h gttabview.h gthometabview.h \
    gtdoctabview.h gtmainsettings.h gtlogindialog.h
SOURCES += gtapplication.cpp gtdocmanager.cpp gtusermanager.cpp \
    gtmainwindow.cpp gttabview.cpp gthometabview.cpp gtdoctabview.cpp \
    gtmainsettings.cpp gtlogindialog.cpp
FORMS += gtmainwindow.ui gthometabview.ui gtlogindialog.ui
RESOURCES = gtapp.qrc
INCLUDEPATH += $$PWD/../../gtbase/message
INCLUDEPATH += $$PWD/../../gtbase/gtbase
INCLUDEPATH += $$PWD/../../gtview
INCLUDEPATH += $$PWD/../../gtsvce/message
INCLUDEPATH += $$PWD/../../gtsvce/gtsvce

CONFIG(debug, debug|release) {
    DESTDIR = ../../build/debug
} else {
    DESTDIR = ../../build/release
}

unix: LIBS += -L$$DESTDIR -lgtbase -lgtview -lgtsvce
