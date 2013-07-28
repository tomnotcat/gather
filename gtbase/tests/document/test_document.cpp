/*
 * Copyright (C) 2013 Tom Wong. All rights reserved.
 */
#include "gtbookmark.h"
#include "gtdocloader.h"
#include "gtdocpage.h"
#include "gtdocument.h"
#include <QtTest/QtTest>

using namespace Gather;

class test_document : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void testDocument();
    void cleanupTestCase();

private:
    GtDocLoader *m_docLoader;
};

void test_document::initTestCase()
{
    m_docLoader = new GtDocLoader(this);

    QDir dir(QCoreApplication::applicationDirPath());
    QVERIFY(dir.cd("loader"));
    QVERIFY(m_docLoader->registerLoaders(dir.absolutePath()) == 1);
}

void test_document::testDocument()
{
    GtDocument *doc = m_docLoader->loadDocument(TEST_PDF_FILE, 0, this);

    QVERIFY(doc && doc->isLoaded());
    QVERIFY(doc->parent() == this);
    QVERIFY(doc->title() == "test.pdf");
    QVERIFY(!doc->fileId().isEmpty());
    QVERIFY(doc->isPageSizeUniform());
    QVERIFY(doc->maxPageSize() == QSize(540, 738));
    QVERIFY(doc->maxPageSize() == doc->minPageSize());
    QVERIFY(doc->pageCount() == 16);

    GtDocPage *page;
    for (int i = 0; i < doc->pageCount(); ++i) {
        page = doc->page(i);
        QVERIFY(page && page->document() == doc);
        QVERIFY(page->index() == i);
        QVERIFY(page->size() == QSize(540, 738));
    }

    page = doc->page(0);
    QVERIFY(page->length() == 2998);
    QVERIFY(page->text()->length() == 2998);

    GtBookmark bm;
    QVERIFY(doc->loadOutline(&bm) == 32);
    QVERIFY(bm.children().size() == 10);
    QVERIFY(bm.children()[0]->children().size() == 0);
    QVERIFY(bm.children()[0]->title() == "SUMMARY");
    QVERIFY(bm.children()[0]->parent() == &bm);
    QVERIFY(bm.children()[0]->dest().type() == GtLinkDest::LinkGoto);
    QVERIFY(bm.children()[0]->dest().page() == 0);
    QVERIFY(bm.children()[3]->children().size() == 3);
    QVERIFY(bm.children()[4]->children().size() == 0);
    QVERIFY(bm.children()[9]->children().size() == 3);
    QVERIFY(bm.children()[9]->children()[0]->children().size() == 5);

    GtBookmark *it = bm.children()[9]->children()[0]->children()[0];
    QVERIFY(it->title() == "C CONCAT/GC");
    QVERIFY(it->dest().type() == GtLinkDest::LinkGoto);
    QVERIFY(it->dest().page() == 12);
    QVERIFY(it->parent() == bm.children()[9]->children()[0]);

    QVERIFY(bm.title() == QString());
    QVERIFY(bm.dest().type() == GtLinkDest::LinkNone);
    QVERIFY(bm.parent() == 0);

    delete doc;
}

void test_document::cleanupTestCase()
{
    delete m_docLoader;

#ifdef GT_DEBUG
    QVERIFY(GtObject::dumpObjects() == 0);
#endif
}

QTEST_MAIN(test_document)
#include "test_document.moc"
