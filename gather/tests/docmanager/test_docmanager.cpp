/*
 * Copyright (C) 2013 Tom Wong. All rights reserved.
 */
#include "gtdocmanager.h"
#include "gtbookmarks.h"
#include "gtdocmeta.h"
#include "gtdocmodel.h"
#include "gtdocnotes.h"
#include "gtdocument.h"
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
    QString fileId;
    QDir dir(QCoreApplication::applicationDirPath());

    QVERIFY(dir.cd("loader"));
    QVERIFY(manager.registerLoaders(dir.absolutePath()) == 1);

    QVERIFY(manager.loadLocalDocument("foo.pdf") == 0);
    QVERIFY(manager.documentCount() == 0);
    QVERIFY(manager.loadLocalDocument(TEST_PDF_FILE));
    QVERIFY(manager.documentCount() == 1);
    model = manager.loadLocalDocument(TEST_PDF_FILE);
    QVERIFY(manager.loadLocalDocument(TEST_PDF_FILE) == model);
    QVERIFY(manager.documentCount() == 1);

    fileId = model->document()->fileId();

    QUndoStack *undoStack = manager.undoStack(model);
    QVERIFY(undoStack);
    QVERIFY(manager.undoStack(model) == undoStack);

    GtDocMeta *meta = model->meta();
    QVERIFY(meta);
    QVERIFY(meta->id() == fileId);
    QVERIFY(manager.loadDocMeta(meta->id()) == meta);

    GtBookmarks *bookmarks = model->bookmarks();
    QVERIFY(bookmarks);
    QVERIFY(meta->bookmarksId() == bookmarks->id());
    QVERIFY(manager.loadBookmarks(bookmarks->id()) == bookmarks);

    GtDocNotes *notes = model->notes();
    QVERIFY(notes);
    QVERIFY(meta->notesId() == notes->id());
    QVERIFY(manager.loadDocNotes(notes->id()) == notes);

    model->ref.ref();
    QVERIFY(manager.cleanUnreferencedObjects() == 0);
    QVERIFY(manager.loadLocalDocument("foo.pdf") == 0);
    model->release();
    QVERIFY(manager.documentCount() == 1);
    QVERIFY(manager.cleanUnreferencedObjects() == 4);
    QVERIFY(manager.documentCount() == 0);

    // auto clean unreferenced objects
    QVERIFY(manager.loadLocalDocument(TEST_PDF_FILE));
    QVERIFY(manager.documentCount() == 1);
    QVERIFY(manager.loadLocalDocument("foo.pdf") == 0);
    QVERIFY(manager.documentCount() == 0);

    QVERIFY(manager.loadLocalDocument(TEST_PDF_FILE) != model);
    QVERIFY(manager.loadDocMeta(fileId) != meta);
    meta = manager.loadDocMeta(fileId);
    QVERIFY(manager.loadBookmarks(meta->bookmarksId()) != bookmarks);
    QVERIFY(manager.loadDocNotes(meta->notesId()) != notes);
}

void test_docmanager::cleanupTestCase()
{
#ifdef GT_DEBUG
    QVERIFY(GtObject::dumpObjects() == 0);
#endif
}

QTEST_MAIN(test_docmanager)
#include "test_docmanager.moc"
