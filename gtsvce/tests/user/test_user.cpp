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
    void stateChanged(int state);

private Q_SLOTS:
    void testLogin();
    void testLogout();
    void cleanupTestCase();

private:
    enum {
        TEST_PORT = 4004
    };

private:
    QList<int> m_states;
};

void test_user::stateChanged(int state)
{
    m_states.append(state);
}

void test_user::testLogin()
{
    GtUserServer server;
    GtUserClient client;
    QHostAddress host(QHostAddress::LocalHost);
    QThread thread;

    QVERIFY(server.listen(host, TEST_PORT));
    server.moveToThread(&thread);

    thread.start();

    connect(&client, SIGNAL(stateChanged(int)),
            this, SLOT(stateChanged(int)));

    QVERIFY(client.error() == GtUserClient::ErrorNone);
    QVERIFY(client.state() == GtUserClient::UnconnectedState);
    QVERIFY(m_states.size() == 0);

    QVERIFY(!client.login("testuser", "testpasswd"));
    QVERIFY(client.error() == GtUserClient::ErrorInvalidState);
    QVERIFY(client.state() == GtUserClient::UnconnectedState);
    QVERIFY(client.session().isEmpty());
    client.clearError();
    QVERIFY(client.error() == GtUserClient::ErrorNone);
    QVERIFY(m_states.size() == 0);

    QVERIFY(client.connect(host, TEST_PORT));
    QVERIFY(client.state() == GtUserClient::ConnectedState);
    QVERIFY(!client.connect(host, TEST_PORT));
    QVERIFY(client.error() == GtUserClient::ErrorInvalidState);
    client.clearError();
    QVERIFY(client.state() == GtUserClient::ConnectedState);
    QVERIFY(m_states.size() == 2);
    QVERIFY(GtUserClient::ConnectingState == m_states[0]);
    QVERIFY(GtUserClient::ConnectedState == m_states[1]);

    m_states.clear();
    client.disconnect();
    QVERIFY(client.state() == GtUserClient::UnconnectedState);
    QVERIFY(m_states.size() == 1);
    QVERIFY(GtUserClient::UnconnectedState == m_states[0]);

    m_states.clear();
    QVERIFY(client.connect(host, TEST_PORT));
    QVERIFY(client.state() == GtUserClient::ConnectedState);
    QVERIFY(m_states.size() == 2);
    QVERIFY(GtUserClient::ConnectingState == m_states[0]);
    QVERIFY(GtUserClient::ConnectedState == m_states[1]);

    m_states.clear();
    QVERIFY(client.login("testuser", "testpasswd"));
    QVERIFY(client.error() == GtUserClient::ErrorNone);
    QVERIFY(client.state() == GtUserClient::LoggedInState);
    QVERIFY(!client.session().isEmpty());
    QVERIFY(!client.login("testuser", "testpasswd"));
    QVERIFY(client.error() == GtUserClient::ErrorInvalidState);
    client.clearError();
    QVERIFY(client.state() == GtUserClient::LoggedInState);
    QVERIFY(m_states.size() == 2);
    QVERIFY(GtUserClient::LoggingInState == m_states[0]);
    QVERIFY(GtUserClient::LoggedInState == m_states[1]);

    m_states.clear();
    client.logout();
    QVERIFY(client.state() == GtUserClient::ConnectedState);
    QVERIFY(client.error() == GtUserClient::ErrorNone);
    QVERIFY(m_states.size() == 1);
    QVERIFY(GtUserClient::ConnectedState == m_states[0]);

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
