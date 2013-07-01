/*
 * Copyright (C) 2013 Tom Wong. All rights reserved.
 */
#include <QtNetwork/QHostAddress>
#include <QtTest/QtTest>
#include "gtclient.h"
#include "gtserver.h"

using namespace Gather;

class test_login: public QObject
{
    Q_OBJECT

private slots:
    void testLogin();
    void testLogout();
};

void test_login::testLogin()
{
    int argc = 0;
    QCoreApplication app(argc, 0);
    GtClient client;
    GtServer server;
    QHostAddress host(QHostAddress::LocalHost);
    const quint16 port = 4004;

    QVERIFY(server.listen(host, port));
    client.login(host, port, "testuser", "testpasswd");

    QVERIFY(app.exec() == 0);
}

void test_login::testLogout()
{
}

QTEST_MAIN(test_login)
#include "test_login.moc"
