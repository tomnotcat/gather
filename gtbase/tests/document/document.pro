CONFIG += testcase
TARGET = test_document
QT = core testlib
SOURCES = test_document.cpp
DEFINES += 'TEST_PDF_FILE=\'\"$$PWD/../test.pdf\"\''

include(../tests.pri)
