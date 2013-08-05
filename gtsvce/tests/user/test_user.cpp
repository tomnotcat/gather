/*
 * Copyright (C) 2013 Tom Wong. All rights reserved.
 */
#include "gtbookmark.h"
#include "gtbookmarks.h"
#include "gtdocmeta.h"
#include "gtdocnote.h"
#include "gtdocnotes.h"
#include "gtuserclient.h"
#include "gtusermessage.pb.h"
#include "gtuserserver.h"
#include <QtNetwork/QHostAddress>
#include <QtTest/QtTest>

using namespace Gather;

class test_user : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void onLogin(int r);
    void onLogout(int r);

private Q_SLOTS:
    void testConvert();
    void testLogin();
    void testLogout();
    void cleanupTestCase();

private:
    enum {
        TEST_PORT = 4004
    };

public:
    inline int exec(int count = 1) { eventCount = count; return app->exec(); }
    inline void exit() { if (--eventCount == 0) app->exit(); }

private:
    int loginResult;
    int logoutReason;
    int eventCount;
    QCoreApplication *app;
    QObject *loginSender;
    QObject *logoutSender;
};

void test_user::onLogin(int r)
{
    loginSender = sender();
    loginResult = r;
    exit();
}

void test_user::onLogout(int r)
{
    logoutSender = sender();
    logoutReason = r;
    exit();
}

void test_user::testConvert()
{
    // document meta
    GtDocMeta dm("meta0");
    GtUserDocMeta udm;
    dm.setBookmarksId("bookmarks0");
    dm.setNotesId("notes0");

    QVERIFY(GtUserClient::convert(dm, udm));
    QVERIFY(udm.id() == "meta0");
    QVERIFY(udm.bookmarks_id() == "bookmarks0");
    QVERIFY(udm.notes_id() == "notes0");
    dm.setBookmarksId("x");
    dm.setNotesId("y");
    QVERIFY(dm.bookmarksId() == "x");
    QVERIFY(dm.notesId() == "y");
    QVERIFY(GtUserClient::convert(udm, dm));
    QVERIFY(dm.id() == "meta0");
    QVERIFY(dm.bookmarksId() == "bookmarks0");
    QVERIFY(dm.notesId() == "notes0");

    // bookmarks
    GtBookmarks bm("id0");
    GtUserBookmarks ubm;

    GtBookmark *b = bm.root();
    b->setTitle("root0");
    b->append(new GtBookmark("sub0", GtLinkDest(10, QPointF(5, 6), 2)));
    b->append(new GtBookmark("sub1", GtLinkDest()));

    QVERIFY(ubm.root().children_size() == 0);
    QVERIFY(GtUserClient::convert(bm, ubm));
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

    QVERIFY(GtUserClient::convert(ubm, bm));
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
    GtUserDocNotes unt;

    GtDocRange range(GtDocPoint(1, QPoint(20, 30)),
                     GtDocPoint(2, QPoint(30, 40)),
                     GtDocRange::TextRange);
    nt.addNote(new GtDocNote(GtDocNote::Underline, range));

    range = GtDocRange(GtDocPoint(3, QPoint(40, 50)),
                       GtDocPoint(4, QPoint(50, 60)),
                       GtDocRange::GeomRange);
    nt.addNote(new GtDocNote(GtDocNote::Highlight, range));

    QVERIFY(GtUserClient::convert(nt, unt));
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
    QVERIFY(GtUserClient::convert(unt, nt));
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

void test_user::testLogin()
{
    int argc = 0;
    QCoreApplication app(argc, 0);
    GtUserServer server;
    GtUserClient client;
    QHostAddress host(QHostAddress::LocalHost);

    this->app = &app;
    connect(&client, SIGNAL(login(int)), this, SLOT(onLogin(int)));

    QVERIFY(server.listen(host, TEST_PORT));

    loginResult = GtUserClient::LoginUnknown;
    client.login(host, TEST_PORT, "testuser", "testpasswd");
    QVERIFY(exec() == 0);
    QVERIFY(loginResult == GtUserClient::LoginSuccess);

    loginResult = GtUserClient::LoginUnknown;
    client.login(host, TEST_PORT, "", "testpasswd");
    QVERIFY(exec() == 0);
    QVERIFY(loginResult == GtUserClient::InvalidUser);

    loginResult = GtUserClient::LoginUnknown;
    client.login(host, TEST_PORT, "testuser", "");
    QVERIFY(exec() == 0);
    QVERIFY(loginResult == GtUserClient::InvalidPasswd);
}

void test_user::testLogout()
{
    int argc = 0;
    QCoreApplication app(argc, 0);
    GtUserClient client1;
    GtUserClient client2;
    GtUserServer server;
    QHostAddress host(QHostAddress::LocalHost);

    this->app = &app;
    connect(&client1, SIGNAL(login(int)), this, SLOT(onLogin(int)));
    connect(&client1, SIGNAL(logout(int)), this, SLOT(onLogout(int)));
    connect(&client2, SIGNAL(login(int)), this, SLOT(onLogin(int)));
    connect(&client2, SIGNAL(logout(int)), this, SLOT(onLogout(int)));

    QVERIFY(server.listen(host, TEST_PORT));

    loginResult = GtUserClient::LoginUnknown;
    logoutReason = GtUserClient::LogoutUnknown;
    logoutSender = 0;
    client1.login(host, TEST_PORT, "testuser", "testpasswd");
    QVERIFY(exec() == 0);
    QVERIFY(loginResult == GtUserClient::LoginSuccess);
    QVERIFY(loginSender == &client1);

    loginResult = GtUserClient::LoginUnknown;
    client2.login(host, TEST_PORT, "testuser", "");
    QVERIFY(exec() == 0);
    QVERIFY(loginResult == GtUserClient::InvalidPasswd);
    QVERIFY(logoutReason == GtUserClient::LogoutUnknown);
    QVERIFY(loginSender == &client2);
    QVERIFY(logoutSender == 0);

    loginResult = GtUserClient::LoginUnknown;
    client2.login(host, TEST_PORT, "testuser", "testpasswd");
    QVERIFY(exec(2) == 0);
    QVERIFY(loginResult == GtUserClient::LoginSuccess);
    QVERIFY(logoutReason == GtUserClient::LogoutRelogin);
    QVERIFY(loginSender == &client2);
    QVERIFY(logoutSender == &client1);

    client2.logout();
    QVERIFY(logoutReason == GtUserClient::LogoutNormal);
}

void test_user::cleanupTestCase()
{
#ifdef GT_DEBUG
    QVERIFY(GtObject::dumpObjects() == 0);
#endif
}

QTEST_MAIN(test_user)
#include "test_user.moc"
