CONFIG += testcase
TARGET = test_docmanager
QT = core testlib
SOURCES = test_docmanager.cpp
DEFINES += 'TEST_PDF_FILE=\'\"$$PWD/../../../gtbase/tests/test.pdf\"\''

include(../tests.pri)
