/*
 * Copyright (C) 2013 Tom Wong. All rights reserved.
 */
#include "gtdocmanager.h"
#include "gtdocmodel.h"
#include <QtTest/QtTest>

using namespace Gather;

class test_docmanager : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void testLocalFile();
    void cleanupTestCase();
};

void test_docmanager::testLocalFile()
{
    GtDocManager manager;
    GtDocModel *model;
    QDir dir(QCoreApplication::applicationDirPath());

    QVERIFY(dir.cd("loader"));
    QVERIFY(manager.registerLoaders(dir.absolutePath()) == 1);

    QVERIFY(manager.loadLocalDocument("foo.pdf") == 0);
    QVERIFY(manager.documentCount() == 0);
    QVERIFY(manager.loadLocalDocument(TEST_PDF_FILE));
    QVERIFY(manager.documentCount() == 1);
    model = manager.loadLocalDocument(TEST_PDF_FILE);
    QVERIFY(manager.documentCount() == 1);
    model->ref.ref();
    QVERIFY(manager.cleanDocuments() == 0);
    QVERIFY(manager.loadLocalDocument("foo.pdf") == 0);
    model->release();
    QVERIFY(manager.documentCount() == 1);
    QVERIFY(manager.cleanDocuments() == 1);
    QVERIFY(manager.documentCount() == 0);

    // auto clean unreferenced documents
    QVERIFY(manager.loadLocalDocument(TEST_PDF_FILE));
    QVERIFY(manager.documentCount() == 1);
    QVERIFY(manager.loadLocalDocument("foo.pdf") == 0);
    QVERIFY(manager.documentCount() == 0);
}

void test_docmanager::cleanupTestCase()
{
#ifdef GT_DEBUG
    QVERIFY(GtObject::dumpObjects() == 0);
#endif
}

QTEST_MAIN(test_docmanager)
#include "test_docmanager.moc"
