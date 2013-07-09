/*
 * Copyright (C) 2013 Tom Wong. All rights reserved.
 */
#include "gtdocument.h"
#include "gtftclient.h"
#include "gtftmessage.pb.h"
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

    QVERIFY(temp.fileId() == "test0415");
    QVERIFY(temp.metaPath() == tempDir.filePath("test0415.meta"));
    QVERIFY(temp.dataPath() == tempDir.filePath("test0415.data"));
    QVERIFY(temp.open(QIODevice::WriteOnly));
    QVERIFY(temp.complete() == 0);
    QVERIFY(temp.size() == 0);

    QFile file(temp.metaPath());
    QVERIFY(file.exists());

    file.setFileName(temp.dataPath());
    QVERIFY(file.exists());

    QVERIFY(temp.remove());
    QVERIFY(!file.exists());

#ifdef Q_WS_WIN
    QFile localFile("test_upload.exe");
#else
    QFile localFile("test_upload");
#endif

    QVERIFY(localFile.open(QIODevice::ReadOnly));
    QString fileId(GtDocument::makeFileId(&localFile));

    temp.setPath(QDir::tempPath(), fileId);
    QVERIFY(temp.open(QIODevice::WriteOnly));

    QFile metaFile(temp.metaPath());
    QVERIFY(metaFile.open(QIODevice::ReadOnly));

    QVERIFY(temp.fileId() == fileId);
    QVERIFY(temp.metaPath() == tempDir.filePath(fileId + ".meta"));
    QVERIFY(temp.dataPath() == tempDir.filePath(fileId + ".data"));
    QVERIFY(temp.complete() == 0);
    QVERIFY(temp.size() == 0);

    QVERIFY(metaFile.size() == 0);
    QVERIFY(temp.write("hello") == 5);
    QVERIFY(temp.size() == 5);

    QVERIFY(temp.seek(100));
    QVERIFY(temp.write("hello") == 5);
    QVERIFY(temp.size() == 105);
    QVERIFY(temp.flush());
    QVERIFY(metaFile.size() > 0);

    QVERIFY(temp.seek(5));
    QVERIFY(temp.write("world") == 5);
    QVERIFY(temp.seek(90));
    QVERIFY(temp.write("hello world, hello world") == 24);
    QVERIFY(temp.size() == 114);
    QVERIFY(temp.seek(80));
    QVERIFY(temp.write("0123456789") == 10);

    QVERIFY(temp.seek(50));
    QVERIFY(temp.write("9876543210") == 10);
    QVERIFY(temp.flush());
    QVERIFY(temp.complete() == 10);

    GtFTTempMeta tempMeta;
    QByteArray metaData(metaFile.readAll());
    QVERIFY(tempMeta.ParseFromArray(metaData.constData(), metaData.size()));
    QVERIFY(QString::fromUtf8(tempMeta.fileid().c_str()) == fileId);
    QVERIFY(tempMeta.datas_size() == 3);
    QVERIFY(tempMeta.datas(0).offset() == 0);
    QVERIFY(tempMeta.datas(0).size() == 10);
    QVERIFY(tempMeta.datas(1).offset() == 50);
    QVERIFY(tempMeta.datas(1).size() == 10);
    QVERIFY(tempMeta.datas(2).offset() == 80);
    QVERIFY(tempMeta.datas(2).size() == 34);

    temp.close();
    QVERIFY(temp.open(QIODevice::ReadOnly));

    char buffer[1024];
    QVERIFY(temp.read(buffer, 10) == 10);
    QVERIFY(strncmp(buffer, "helloworld", 10) == 0);
    QVERIFY(temp.seek(50));
    QVERIFY(temp.read(buffer, 10) == 10);
    QVERIFY(strncmp(buffer, "9876543210", 10) == 0);
    QVERIFY(temp.complete() == 10);

    metaFile.close();
    QVERIFY(temp.remove());

    QVERIFY(localFile.seek(0));
    QVERIFY(temp.open(QIODevice::WriteOnly | QIODevice::Truncate));
    qint64 length;
    do {
        length = localFile.read(buffer, sizeof(buffer));
        if (length > 0) {
            temp.write(buffer, length);
        }
    } while (length > 0);

    QVERIFY(temp.flush());
    QVERIFY(temp.complete() == localFile.size());

    temp.close();
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