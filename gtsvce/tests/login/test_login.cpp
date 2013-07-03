/*
 * Copyright (C) 2013 Tom Wong. All rights reserved.
 */
#include <QtNetwork/QHostAddress>
#include <QtTest/QtTest>
#include "gtclient.h"
#include "gtuserserver.h"

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
    GtClient client;
    QHostAddress host(QHostAddress::LocalHost);

    this->app = &app;
    connect(&client, SIGNAL(onLogin(int)), this, SLOT(onLogin(int)));

    QVERIFY(server.listen(host, TEST_PORT));

    loginResult = GtClient::LoginUnknown;
    client.login(host, TEST_PORT, "testuser", "testpasswd");
    QVERIFY(exec() == 0);
    QVERIFY(loginResult == GtClient::LoginSuccess);

    loginResult = GtClient::LoginUnknown;
    client.login(host, TEST_PORT, "", "testpasswd");
    QVERIFY(exec() == 0);
    QVERIFY(loginResult == GtClient::InvalidUser);

    loginResult = GtClient::LoginUnknown;
    client.login(host, TEST_PORT, "testuser", "");
    QVERIFY(exec() == 0);
    QVERIFY(loginResult == GtClient::InvalidPasswd);
}

void test_login::testLogout()
{
    int argc = 0;
    QCoreApplication app(argc, 0);
    GtClient client1;
    GtClient client2;
    GtUserServer server;
    QHostAddress host(QHostAddress::LocalHost);

    this->app = &app;
    connect(&client1, SIGNAL(onLogin(int)), this, SLOT(onLogin(int)));
    connect(&client1, SIGNAL(onLogout(int)), this, SLOT(onLogout(int)));
    connect(&client2, SIGNAL(onLogin(int)), this, SLOT(onLogin(int)));
    connect(&client2, SIGNAL(onLogout(int)), this, SLOT(onLogout(int)));

    QVERIFY(server.listen(host, TEST_PORT));

    loginResult = GtClient::LoginUnknown;
    logoutReason = GtClient::LogoutUnknown;
    logoutSender = 0;
    client1.login(host, TEST_PORT, "testuser", "testpasswd");
    QVERIFY(exec() == 0);
    QVERIFY(loginResult == GtClient::LoginSuccess);
    QVERIFY(loginSender == &client1);

    loginResult = GtClient::LoginUnknown;
    client2.login(host, TEST_PORT, "testuser", "");
    QVERIFY(exec() == 0);
    QVERIFY(loginResult == GtClient::InvalidPasswd);
    QVERIFY(logoutReason == GtClient::LogoutUnknown);
    QVERIFY(loginSender == &client2);
    QVERIFY(logoutSender == 0);

    loginResult = GtClient::LoginUnknown;
    client2.login(host, TEST_PORT, "testuser", "testpasswd");
    QVERIFY(exec(2) == 0);
    QVERIFY(loginResult == GtClient::LoginSuccess);
    QVERIFY(logoutReason == GtClient::LogoutRelogin);
    QVERIFY(loginSender == &client2);
    QVERIFY(logoutSender == &client1);

    client2.logout();
    QVERIFY(logoutReason == GtClient::LogoutNormal);
}

void test_login::cleanupTestCase()
{
#ifdef GT_DEBUG
    QVERIFY(GtObject::dumpObjects() == 0);
#endif
}

QTEST_MAIN(test_login)
#include "test_login.moc"
