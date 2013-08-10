/*
 * Copyright (C) 2013 Tom Wong. All rights reserved.
 */
#include "gtbookmark.h"
#include "gtbookmarks.h"
#include "gtdocmeta.h"
#include "gtdocnote.h"
#include "gtdocnotes.h"
#include "gtuserclient.h"
#include "gtuserserver.h"
#include <QtNetwork/QHostAddress>
#include <QtTest/QtTest>

using namespace Gather;

class test_user : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void testLogin();
    void testLogout();
    void cleanupTestCase();

private:
    enum {
        TEST_PORT = 4004
    };
};

void test_user::testLogin()
{
    GtUserServer server;
    GtUserClient client;
    QHostAddress host(QHostAddress::LocalHost);
    QThread thread;

    QVERIFY(server.listen(host, TEST_PORT));
    server.moveToThread(&thread);

    thread.start();

    QVERIFY(client.error() == GtUserClient::ErrorNone);
    QVERIFY(client.state() == GtUserClient::UnconnectedState);

    QVERIFY(!client.login("testuser", "testpasswd"));
    QVERIFY(client.error() == GtUserClient::ErrorInvalidState);
    QVERIFY(client.state() == GtUserClient::UnconnectedState);
    QVERIFY(client.sessionId().isEmpty());
    client.clearError();
    QVERIFY(client.error() == GtUserClient::ErrorNone);

    QVERIFY(client.connect(host, TEST_PORT));
    QVERIFY(client.state() == GtUserClient::ConnectedState);
    QVERIFY(!client.connect(host, TEST_PORT));
    QVERIFY(client.error() == GtUserClient::ErrorInvalidState);
    client.clearError();
    QVERIFY(client.state() == GtUserClient::ConnectedState);

    client.disconnect();
    QVERIFY(client.state() == GtUserClient::UnconnectedState);

    QVERIFY(client.connect(host, TEST_PORT));
    QVERIFY(client.state() == GtUserClient::ConnectedState);

    QVERIFY(client.login("testuser", "testpasswd"));
    QVERIFY(client.error() == GtUserClient::ErrorNone);
    QVERIFY(client.state() == GtUserClient::LoggedInState);
    QVERIFY(!client.sessionId().isEmpty());
    QVERIFY(!client.login("testuser", "testpasswd"));
    QVERIFY(client.error() == GtUserClient::ErrorInvalidState);
    client.clearError();
    QVERIFY(client.state() == GtUserClient::LoggedInState);

    client.logout();
    QVERIFY(client.state() == GtUserClient::ConnectedState);
    QVERIFY(client.error() == GtUserClient::ErrorNone);

    thread.quit();
    thread.wait();
}

void test_user::testLogout()
{
}

void test_user::cleanupTestCase()
{
#ifdef GT_DEBUG
    QVERIFY(GtObject::dumpObjects() == 0);
#endif
}

QTEST_MAIN(test_user)
#include "test_user.moc"
