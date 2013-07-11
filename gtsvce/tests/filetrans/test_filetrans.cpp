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

class test_filetrans: public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void onUpload(const QString &fileId);

private Q_SLOTS:
    void testTempFile();
    void testNormalUpload();
    void testDamageUpload();
    void cleanupTestCase();

private:
    enum {
        TEST_PORT = 4004
    };

private:
    QString uploaded;
};

void test_filetrans::onUpload(const QString &fileId)
{
    QVERIFY(uploaded.isNull());
    uploaded = fileId;
}

void test_filetrans::testTempFile()
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
    QFile localFile("test_filetrans.exe");
#else
    QFile localFile("test_filetrans");
#endif

    QVERIFY(localFile.open(QIODevice::ReadOnly));
    QString fileId(GtDocument::makeFileId(&localFile));

    temp.setPath(QDir::tempPath(), fileId);
    QVERIFY(temp.open(QIODevice::WriteOnly));
    QVERIFY(temp.temps_size() == 0);

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
    QVERIFY(temp.temps_size() == 1);
    QVERIFY(temp.temps(0).offset() == 0);
    QVERIFY(temp.temps(0).size() == 5);

    QVERIFY(temp.seek(100));
    QVERIFY(temp.write("hello") == 5);
    QVERIFY(temp.size() == 105);
    QVERIFY(temp.flush());
    QVERIFY(metaFile.size() > 0);
    QVERIFY(temp.temps_size() == 2);
    QVERIFY(temp.temps(0).offset() == 0);
    QVERIFY(temp.temps(0).size() == 5);
    QVERIFY(temp.temps(1).offset() == 100);
    QVERIFY(temp.temps(1).size() == 5);

    QVERIFY(temp.seek(5));
    QVERIFY(temp.write("world") == 5);
    QVERIFY(temp.temps_size() == 2);
    QVERIFY(temp.temps(0).offset() == 0);
    QVERIFY(temp.temps(0).size() == 10);
    QVERIFY(temp.temps(1).offset() == 100);
    QVERIFY(temp.temps(1).size() == 5);

    QVERIFY(temp.seek(90));
    QVERIFY(temp.write("hello world, hello world") == 24);
    QVERIFY(temp.size() == 114);
    QVERIFY(temp.seek(80));
    QVERIFY(temp.write("0123456789") == 10);
    QVERIFY(temp.temps_size() == 2);
    QVERIFY(temp.temps(0).offset() == 0);
    QVERIFY(temp.temps(0).size() == 10);
    QVERIFY(temp.temps(1).offset() == 80);
    QVERIFY(temp.temps(1).size() == 34);

    QVERIFY(temp.seek(50));
    QVERIFY(temp.write("9876543210") == 10);
    QVERIFY(temp.flush());
    QVERIFY(temp.complete() == 10);
    QVERIFY(temp.temps_size() == 3);
    QVERIFY(temp.temps(0).offset() == 0);
    QVERIFY(temp.temps(0).size() == 10);
    QVERIFY(temp.temps(1).offset() == 50);
    QVERIFY(temp.temps(1).size() == 10);
    QVERIFY(temp.temps(2).offset() == 80);
    QVERIFY(temp.temps(2).size() == 34);

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
    QVERIFY(temp.temps_size() == 3);
    QVERIFY(temp.temps(0).offset() == 0);
    QVERIFY(temp.temps(0).size() == 10);
    QVERIFY(temp.temps(1).offset() == 50);
    QVERIFY(temp.temps(1).size() == 10);
    QVERIFY(temp.temps(2).offset() == 80);
    QVERIFY(temp.temps(2).size() == 34);

    char buffer[1024];
    QVERIFY(temp.read(buffer, 10) == 10);
    QVERIFY(strncmp(buffer, "helloworld", 10) == 0);
    QVERIFY(temp.seek(50));
    QVERIFY(temp.read(buffer, 10) == 10);
    QVERIFY(strncmp(buffer, "9876543210", 10) == 0);
    QVERIFY(temp.complete() == 10);
    QVERIFY(temp.complete(50) == 60);
    QVERIFY(temp.complete(80) == 114);
    QVERIFY(temp.complete(100) == 114);

    metaFile.close();
    temp.close();

    QVERIFY(localFile.seek(0));
    QVERIFY(temp.open(QIODevice::ReadWrite | QIODevice::Truncate));
    QVERIFY(temp.temps_size() == 0);

    qint64 length;
    do {
        length = localFile.read(buffer, sizeof(buffer));
        QVERIFY(temp.write(buffer, length) == length);
    } while (length > 0);

    QVERIFY(temp.flush());
    QVERIFY(temp.complete() == localFile.size());
    QVERIFY(temp.complete(10) == localFile.size());

    QVERIFY(temp.seek(0));
    QVERIFY(GtDocument::makeFileId(&temp) == temp.fileId());

    QVERIFY(temp.remove());
    temp.close();
    localFile.close();
}

void test_filetrans::testNormalUpload()
{
    GtFTServer server;
    GtFTClient client;
    QHostAddress host(QHostAddress::LocalHost);
    QThread thread;

    uploaded = QString();
    QVERIFY(server.listen(host, TEST_PORT));
    server.moveToThread(&thread);
    connect(&server,
            SIGNAL(uploaded(const QString&)),
            this,
            SLOT(onUpload(const QString&)),
            Qt::DirectConnection);

    thread.start();

#ifdef Q_WS_WIN
    QFile localFile("test_filetrans.exe");
#else
    QFile localFile("test_filetrans");
#endif

    QVERIFY(localFile.open(QIODevice::ReadOnly));
    QString fileId(GtDocument::makeFileId(&localFile));

    client.setFileInfo(fileId, host, TEST_PORT, "");
    QVERIFY(!client.open(QIODevice::WriteOnly));
    QVERIFY(client.error() == GtFTClient::InvalidSession);

    QVERIFY(client.fileId() == fileId);

    client.setFileInfo(fileId, host, TEST_PORT, "testsession");
    QVERIFY(client.open(QIODevice::WriteOnly));
    QVERIFY(!client.open(QIODevice::WriteOnly));
    QVERIFY(client.size() == 0);

    char buffer[1024];
    qint64 length;
    qint64 size = 0;

    QVERIFY(localFile.seek(0));
    do {
        length = localFile.read(buffer, sizeof(buffer));
        QVERIFY(client.write(buffer, length) == length);
        size += length;
        QVERIFY(client.size() == size);
        QVERIFY(client.uploaded() == size);

        if (size < localFile.size())
            QVERIFY(!client.complete());
        else
            QVERIFY(client.complete());
    } while (length > 0);

    QVERIFY(localFile.size() == size);

    client.close();
    server.close();
    localFile.close();

    thread.quit();
    thread.wait();

    QVERIFY(uploaded == fileId);
    GtFTTemp temp(QDir::tempPath(), fileId);
    QVERIFY(temp.remove());
}

void test_filetrans::testDamageUpload()
{
    GtFTServer server;
    GtFTClient client;
    QHostAddress host(QHostAddress::LocalHost);
    QThread thread;

    uploaded = QString();
    QVERIFY(server.listen(host, TEST_PORT));
    server.moveToThread(&thread);
    connect(&server,
            SIGNAL(uploaded(const QString&)),
            this,
            SLOT(onUpload(const QString&)),
            Qt::DirectConnection);

    thread.start();

#ifdef Q_WS_WIN
    QFile localFile("test_filetrans.exe");
#else
    QFile localFile("test_filetrans");
#endif

    QVERIFY(localFile.open(QIODevice::ReadOnly));
    QString fileId(GtDocument::makeFileId(&localFile));

    client.setFileInfo(fileId, host, TEST_PORT, "testsession");
    QVERIFY(client.open(QIODevice::WriteOnly));

    char buffer[1024];
    qint64 length;
    qint64 size = 0;
    qint64 half = localFile.size() / 2;
    qint64 cutOffset = 0;
    qint64 cutLength = 0;

    QVERIFY(localFile.seek(0));
    do {
        length = localFile.read(buffer, sizeof(buffer));

        if (size < half) {
            QVERIFY(client.write(buffer, length) == length);
            size += length;
        }
        else {
            cutOffset = size;
            cutLength = length;
            QVERIFY(client.seek(size + length));
            size = 0;
        }
    } while (length > 0);

    QVERIFY(client.size() == localFile.size());
    QVERIFY(client.uploaded() == localFile.size() - cutLength);
    QVERIFY(cutOffset > 0 && cutLength > 0);
    QVERIFY(!client.complete());

    client.close();
    QVERIFY(client.open(QIODevice::WriteOnly));
    QVERIFY(client.size() == localFile.size());
    QVERIFY(client.uploaded() == localFile.size() - cutLength);
    QVERIFY(!client.complete());

    QVERIFY(localFile.seek(cutOffset));
    QVERIFY(localFile.read(buffer, sizeof(buffer)) == cutLength);
    QVERIFY(client.seek(cutOffset));
    QVERIFY(client.write(buffer, cutLength) == cutLength);
    QVERIFY(client.uploaded() == localFile.size());
    QVERIFY(client.complete());

    client.close();
    server.close();
    localFile.close();

    thread.quit();
    thread.wait();

    QVERIFY(uploaded == fileId);
    GtFTTemp temp(QDir::tempPath(), fileId);
    QVERIFY(temp.remove());
}

void test_filetrans::cleanupTestCase()
{
#ifdef GT_DEBUG
    QVERIFY(GtObject::dumpObjects() == 0);
#endif
}

QTEST_MAIN(test_filetrans)
#include "test_filetrans.moc"
