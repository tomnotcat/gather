/*
 * Copyright (C) 2013 Tom Wong. All rights reserved.
 */
#include "gtuserclient.h"
#include "gtuserserver.h"
#include <QtNetwork/QHostAddress>
#include <QtTest/QtTest>

using namespace Gather;

class test_login: public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void onLogin(int r);
    void onLogout(int r);

private Q_SLOTS:
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

void test_login::onLogin(int r)
{
    loginSender = sender();
    loginResult = r;
    exit();
}

void test_login::onLogout(int r)
{
    logoutSender = sender();
    logoutReason = r;
    exit();
}

void test_login::testLogin()
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

void test_login::testLogout()
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

void test_login::cleanupTestCase()
{
#ifdef GT_DEBUG
    QVERIFY(GtObject::dumpObjects() == 0);
#endif
}

QTEST_MAIN(test_login)
#include "test_login.moc"
