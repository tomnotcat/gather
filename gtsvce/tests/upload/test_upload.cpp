/*
 * Copyright (C) 2013 Tom Wong. All rights reserved.
 */
#include "gtdocument.h"
#include "gtftclient.h"
#include "gtftserver.h"
#include <QtNetwork/QHostAddress>
#include <QtTest/QtTest>

using namespace Gather;

class test_upload: public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void onConnection(int r);

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
    int connectionCode;
    int eventCount;
    QCoreApplication *app;
};

void test_upload::onConnection(int r)
{
    connectionCode = r;
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
    connect(&client, SIGNAL(connection(int)), this, SLOT(onConnection(int)));

    QVERIFY(server.listen(host, TEST_PORT));

#ifdef Q_WS_WIN
    QFile localFile("test_upload.exe");
#else
    QFile localFile("test_upload");
#endif

    QVERIFY(localFile.open(QIODevice::ReadOnly));
    QString fileId(GtDocument::makeFileId(&localFile));

    connectionCode = GtFTClient::UnknownError;
    client.setFileInfo(fileId, host, TEST_PORT, "");
    QVERIFY(client.open(QIODevice::WriteOnly));

    exec();
    QVERIFY(GtFTClient::InvalidSession == connectionCode);

    QVERIFY(client.fileId() == fileId);
    QVERIFY(client.size() == 0);

    client.setFileInfo(fileId, host, TEST_PORT, "testsession");
    QVERIFY(client.open(QIODevice::WriteOnly));

    exec();
    QVERIFY(GtFTClient::OpenSuccess == connectionCode);

    client.close();
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
