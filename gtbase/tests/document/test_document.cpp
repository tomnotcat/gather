/*
 * Copyright (C) 2013 Tom Wong. All rights reserved.
 */
#include "gtbookmark.h"
#include "gtbookmarks.h"
#include "gtdocloader.h"
#include "gtdocmessage.pb.h"
#include "gtdocmeta.h"
#include "gtdocnote.h"
#include "gtdocnotes.h"
#include "gtdocpage.h"
#include "gtdocument.h"
#include <QtTest/QtTest>

using namespace Gather;

class test_document : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void testSerialize();
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

void test_document::testSerialize()
{
    // document meta
    GtDocMeta dm("meta0");
    GtDocMetaMsg udm;

    dm.setBookmarksId("bookmarks0");
    dm.setNotesId("notes0");

    dm.serialize(udm);
    QVERIFY(udm.id() == "meta0");
    QVERIFY(udm.bookmarks_id() == "bookmarks0");
    QVERIFY(udm.notes_id() == "notes0");
    dm.setBookmarksId("x");
    dm.setNotesId("y");
    QVERIFY(dm.bookmarksId() == "x");
    QVERIFY(dm.notesId() == "y");
    QVERIFY(dm.deserialize(udm));
    QVERIFY(dm.id() == "meta0");
    QVERIFY(dm.bookmarksId() == "bookmarks0");
    QVERIFY(dm.notesId() == "notes0");

    // bookmarks
    GtBookmarks bm("id0");
    GtBookmarksMsg ubm;

    GtBookmark *b = bm.root();
    b->setTitle("root0");
    b->append(new GtBookmark("sub0", GtLinkDest(10, QPointF(5, 6), 2)));
    b->append(new GtBookmark("sub1", GtLinkDest()));

    QVERIFY(ubm.root().children_size() == 0);
    bm.serialize(ubm);
    QVERIFY(ubm.id() == "id0");
    QVERIFY(ubm.root().title() == "root0");
    QVERIFY(ubm.root().children_size() == 2);
    QVERIFY(ubm.root().children(0).title() == "sub0");
    QVERIFY(ubm.root().children(0).dest().type() == GtLinkDest::ScrollTo);
    QVERIFY(ubm.root().children(0).dest().x() == 5);
    QVERIFY(ubm.root().children(0).dest().y() == 6);
    QVERIFY(ubm.root().children(0).dest().zoom() == 2);
    QVERIFY(!ubm.root().children(0).dest().has_uri());
    QVERIFY(ubm.root().children(1).title() == "sub1");
    QVERIFY(ubm.root().children(1).dest().type() == GtLinkDest::LinkNone);
    QVERIFY(!ubm.root().children(1).dest().has_x());
    QVERIFY(!ubm.root().children(1).dest().has_y());
    QVERIFY(!ubm.root().children(1).dest().has_zoom());
    QVERIFY(!ubm.root().children(1).dest().has_uri());

    QVERIFY(b->title() == "root0");
    QVERIFY(b->children().size() == 2);
    b->setTitle("");
    b->clearChildren();
    QVERIFY(b->title() == "");
    QVERIFY(b->children().size() == 0);

    QVERIFY(bm.deserialize(ubm));
    QVERIFY(b->title() == "root0");
    QVERIFY(b->children().size() == 2);
    QVERIFY(b->children()[0]->title() == "sub0");
    QVERIFY(b->children()[0]->dest().type() == GtLinkDest::ScrollTo);
    QVERIFY(b->children()[0]->dest().point() == QPoint(5, 6));
    QVERIFY(b->children()[0]->dest().zoom() == 2);
    QVERIFY(b->children()[1]->title() == "sub1");
    QVERIFY(b->children()[1]->dest().type() == GtLinkDest::LinkNone);

    // document notes
    GtDocNotes nt("id1");
    GtDocNotesMsg unt;

    GtDocRange range(GtDocPoint(1, QPoint(20, 30)),
                     GtDocPoint(2, QPoint(30, 40)),
                     GtDocRange::TextRange);
    nt.addNote(new GtDocNote(GtDocNote::Underline, range));

    range = GtDocRange(GtDocPoint(3, QPoint(40, 50)),
                       GtDocPoint(4, QPoint(50, 60)),
                       GtDocRange::GeomRange);
    nt.addNote(new GtDocNote(GtDocNote::Highlight, range));

    nt.serialize(unt);
    QVERIFY(unt.id() == "id1");
    QVERIFY(unt.notes_size() == 2);
    QVERIFY(unt.notes(0).type() == GtDocNote::Underline);
    QVERIFY(unt.notes(0).range().type() == GtDocRange::TextRange);
    QVERIFY(unt.notes(0).range().begin_page() == 1);
    QVERIFY(unt.notes(0).range().begin_x() == 20);
    QVERIFY(unt.notes(0).range().begin_y() == 30);
    QVERIFY(unt.notes(0).range().end_page() == 2);
    QVERIFY(unt.notes(0).range().end_x() == 30);
    QVERIFY(unt.notes(0).range().end_y() == 40);
    QVERIFY(unt.notes(1).type() == GtDocNote::Highlight);
    QVERIFY(unt.notes(1).range().type() == GtDocRange::GeomRange);
    QVERIFY(unt.notes(1).range().begin_page() == 3);
    QVERIFY(unt.notes(1).range().begin_x() == 40);
    QVERIFY(unt.notes(1).range().begin_y() == 50);
    QVERIFY(unt.notes(1).range().end_page() == 4);
    QVERIFY(unt.notes(1).range().end_x() == 50);
    QVERIFY(unt.notes(1).range().end_y() == 60);

    nt.clearAll();
    QVERIFY(nt.allNotes().size() == 0);
    QVERIFY(nt.pageCount() == 0);
    QVERIFY(nt.deserialize(unt));
    QVERIFY(nt.allNotes().size() == 2);
    QVERIFY(nt.pageCount() == 5);
    QVERIFY(nt.allNotes()[0]->type() == GtDocNote::Underline);
    QVERIFY(nt.allNotes()[0]->range().type() == GtDocRange::TextRange);
    QVERIFY(nt.allNotes()[0]->range().begin() == GtDocPoint(1, QPoint(20, 30)));
    QVERIFY(nt.allNotes()[0]->range().end() == GtDocPoint(2, QPoint(30, 40)));
    QVERIFY(nt.allNotes()[1]->type() == GtDocNote::Highlight);
    QVERIFY(nt.allNotes()[1]->range().type() == GtDocRange::GeomRange);
    QVERIFY(nt.allNotes()[1]->range().begin() == GtDocPoint(3, QPoint(40, 50)));
    QVERIFY(nt.allNotes()[1]->range().end() == GtDocPoint(4, QPoint(50, 60)));
}

void test_document::testDocument()
{
    GtDocument *doc = m_docLoader->loadDocument(TEST_PDF_FILE, 0);

    QVERIFY(doc && doc->isLoaded());
    QVERIFY(doc->parent() == 0);
    QVERIFY(doc->thread() == QThread::currentThread());
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
    QVERIFY(bm.index() == 0);
    QVERIFY(bm.children().size() == 10);
    QVERIFY(bm.children()[0]->prev() == 0);
    QVERIFY(bm.children()[0]->next() == bm.children()[1]);
    QVERIFY(bm.children()[1]->prev() == bm.children()[0]);
    QVERIFY(bm.children()[0]->children().size() == 0);
    QVERIFY(bm.children()[0]->title() == "SUMMARY");
    QVERIFY(bm.children()[0]->parent() == &bm);
    QVERIFY(bm.children()[0]->dest().type() == GtLinkDest::ScrollTo);
    QVERIFY(bm.children()[0]->dest().page() == 0);
    QVERIFY(bm.children()[0]->index() == 0);
    QVERIFY(bm.children()[3]->children().size() == 3);
    QVERIFY(bm.children()[3]->index() == 3);
    QVERIFY(bm.children()[4]->children().size() == 0);
    QVERIFY(bm.children()[4]->index() == 4);
    QVERIFY(bm.children()[9]->children().size() == 3);
    QVERIFY(bm.children()[9]->index() == 9);
    QVERIFY(bm.children()[9]->next() == 0);
    QVERIFY(bm.children()[9]->prev() == bm.children()[8]);
    QVERIFY(bm.children()[8]->next() == bm.children()[9]);
    QVERIFY(bm.children()[9]->children()[0]->children().size() == 5);

    GtBookmark *it = bm.children()[9]->children()[0]->children()[0];
    QVERIFY(it->title() == "C CONCAT/GC");
    QVERIFY(it->dest().type() == GtLinkDest::ScrollTo);
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
