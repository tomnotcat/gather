/*
 * Copyright (C) 2013 Tom Wong. All rights reserved.
 */
#include <QtNetwork/QHostAddress>
#include <QtTest/QtTest>
#include "gtftupload.h"
#include "gtftdownload.h"
#include "gtftserver.h"

using namespace Gather;

class test_upload: public QObject
{
    Q_OBJECT

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
    int eventCount;
    QCoreApplication *app;
};

void test_upload::testNormal()
{
    int argc = 0;
    QCoreApplication app(argc, 0);
    GtFTServer server;
    GtFTUpload *upload;
    QHostAddress host(QHostAddress::LocalHost);

    this->app = &app;

    QVERIFY(server.listen(host, TEST_PORT));

    QThreadPool *threadPool = QThreadPool::globalInstance();
#ifdef Q_WS_WIN
    QFile file("test_upload.exe");
#else
    QFile file("test_upload");
#endif

    QVERIFY(file.open(QIODevice::ReadOnly));
    upload = new GtFTUpload(host, TEST_PORT, "", "", &file);
    threadPool->start(upload);

    exec();
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
