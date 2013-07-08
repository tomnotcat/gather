/*
 * Copyright (C) 2013 Tom Wong. All rights reserved.
 */
#include "gtdocument.h"
#include "gtftclient.h"
#include "gtftfile.h"
#include "gtftserver.h"
#include <QtNetwork/QHostAddress>
#include <QtTest/QtTest>

using namespace Gather;

class test_upload: public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void onLogin(int r);
    void onLogout(int r);

private Q_SLOTS:
    void testNormal();
    void testBlocked();
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
};

void test_upload::onLogin(int r)
{
    loginResult = r;
    exit();
}

void test_upload::onLogout(int r)
{
    logoutReason = r;
    exit();
}

void test_upload::testNormal()
{
    int argc = 0;
    QCoreApplication app(argc, 0);
    GtFTServer server;
    GtFTClient client;
    QHostAddress host(QHostAddress::LocalHost);

    this->app = &app;
    connect(&client, SIGNAL(login(int)), this, SLOT(onLogin(int)));
    connect(&client, SIGNAL(logout(int)), this, SLOT(onLogout(int)));

    QVERIFY(server.listen(host, TEST_PORT));

#ifdef Q_WS_WIN
    QFile localFile("test_upload.exe");
#else
    QFile localFile("test_upload");
#endif

    QVERIFY(localFile.open(QIODevice::ReadOnly));

    loginResult = GtFTClient::LoginUnknown;
    logoutReason = GtFTClient::LogoutUnknown;
    client.login(host, TEST_PORT, "", "");
    exec();
    QVERIFY(GtFTClient::InvalidSession == loginResult);
    QVERIFY(GtFTClient::LogoutUnknown == logoutReason);

    client.logout();
    QVERIFY(GtFTClient::InvalidSession == loginResult);
    QVERIFY(GtFTClient::LogoutNormal == logoutReason);

    QString fileId(GtDocument::makeFileId(&localFile));
    QVERIFY(client.openFile(fileId) == 0);

    client.login(host, TEST_PORT, "testsession", "testsecret");
    exec();
    QVERIFY(GtFTClient::LoginSuccess == loginResult);

    GtFTFile *file = client.openFile(fileId);
    QVERIFY(file->fileId() == fileId);
    QVERIFY(client.openFile(fileId) != file);

    file->close();
    localFile.close();
}

void test_upload::testBlocked()
{
    int argc = 0;
    QCoreApplication app(argc, 0);
    GtFTServer server;
    QHostAddress host(QHostAddress::LocalHost);

    this->app = &app;

    QVERIFY(server.listen(host, TEST_PORT));
}

void test_upload::cleanupTestCase()
{
#ifdef GT_DEBUG
    QVERIFY(GtObject::dumpObjects() == 0);
#endif
}

QTEST_MAIN(test_upload)
#include "test_upload.moc"
