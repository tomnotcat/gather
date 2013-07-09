/*
 * Copyright (C) 2013 Tom Wong. All rights reserved.
 */
#include "gtdocument.h"
#include "gtftclient.h"
#include "gtftserver.h"
#include "gtfttemp.h"
#include <QtNetwork/QHostAddress>
#include <QtTest/QtTest>

using namespace Gather;

class test_upload: public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void testTempFile();
    void testNormal();
    void testBlocked();
    void cleanupTestCase();

private:
    enum {
        TEST_PORT = 4004
    };
};

void test_upload::testTempFile()
{

    QDir tempDir(QDir::temp());
    GtFTTemp temp(QDir::tempPath(), "test0415");

    QVERIFY(temp.metaPath() == tempDir.filePath("test0415.meta"));
    QVERIFY(temp.dataPath() == tempDir.filePath("test0415.data"));
    QVERIFY(!temp.check());

#ifdef Q_WS_WIN
    QFile localFile("test_upload.exe");
#else
    QFile localFile("test_upload");
#endif

    QVERIFY(localFile.open(QIODevice::ReadOnly));
    QString fileId(GtDocument::makeFileId(&localFile));

    temp.setPath(QDir::tempPath(), fileId);

    QVERIFY(temp.metaPath() == tempDir.filePath(fileId + ".meta"));
    QVERIFY(temp.dataPath() == tempDir.filePath(fileId + ".data"));
    QVERIFY(!temp.check());

    localFile.close();
}

void test_upload::testNormal()
{
    GtFTServer server;
    GtFTClient client;
    QHostAddress host(QHostAddress::LocalHost);
    QThread thread;

    QVERIFY(server.listen(host, TEST_PORT));
    server.moveToThread(&thread);

    thread.start();

#ifdef Q_WS_WIN
    QFile localFile("test_upload.exe");
#else
    QFile localFile("test_upload");
#endif

    QVERIFY(localFile.open(QIODevice::ReadOnly));
    QString fileId(GtDocument::makeFileId(&localFile));

    client.setFileInfo(fileId, host, TEST_PORT, "");
    QVERIFY(!client.open(QIODevice::WriteOnly));
    QVERIFY(client.error() == GtFTClient::InvalidSession);

    QVERIFY(client.fileId() == fileId);
    QVERIFY(client.size() == 0);

    client.setFileInfo(fileId, host, TEST_PORT, "testsession");
    QVERIFY(client.open(QIODevice::WriteOnly));
    QVERIFY(!client.open(QIODevice::WriteOnly));

    client.close();
    localFile.close();

    thread.quit();
    thread.wait();
}

void test_upload::testBlocked()
{
    GtFTServer server;
    QHostAddress host(QHostAddress::LocalHost);

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
