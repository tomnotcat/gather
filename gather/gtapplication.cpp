/*
 * Copyright (C) 2013 Tom Wong. All rights reserved.
 */
#include "gtapplication.h"
#include "gtdocloader.h"
#include "gtmainsettings.h"
#include "gtmainwindow.h"
#include "gtuserclient.h"
#include <QtCore/QDebug>
#include <QtCore/QDir>
#include <QtCore/QStringList>
#include <QtCore/QTextStream>
#include <QtCore/QThread>
#include <QtCore/QTimer>
#include <QtGui/QFileOpenEvent>
#include <QtGui/QIcon>
#include <QtNetwork/QLocalServer>
#include <QtNetwork/QLocalSocket>

GT_BEGIN_NAMESPACE

GtApplication::GtApplication(int &argc, char **argv)
    : QApplication(argc, argv)
    , m_localServer(0)
{
    QCoreApplication::setOrganizationName(QLatin1String("Clue Network"));
    QCoreApplication::setApplicationName(QLatin1String("Gather"));
    QCoreApplication::setApplicationVersion(QLatin1String("0.1"));

#ifdef Q_WS_QWS
    // Use a different server name for QWS so we can run an X11
    // browser and a QWS browser in parallel on the same machine for
    // debugging
    QString serverName = QCoreApplication::applicationName() + QLatin1String("_qws");
#else
    QString serverName = QCoreApplication::applicationName();
#endif

    QLocalSocket socket;
    socket.connectToServer(serverName);
    if (socket.waitForConnected(500)) {
        QTextStream stream(&socket);
        QStringList args = QCoreApplication::arguments();
        if (args.count() > 1)
            stream << args.last();
        else
            stream << QString();
        stream.flush();
        socket.waitForBytesWritten();
        return;
    }

#if defined(Q_WS_MAC)
    QApplication::setQuitOnLastWindowClosed(false);
#else
    QApplication::setQuitOnLastWindowClosed(true);
#endif

    m_localServer = new QLocalServer(this);
    connect(m_localServer,
            SIGNAL(newConnection()),
            this, SLOT(newLocalSocketConnection()));
    if (!m_localServer->listen(serverName)) {
        if (m_localServer->serverError() == QAbstractSocket::AddressInUseError
            && QFile::exists(m_localServer->serverName()))
        {
            QFile::remove(m_localServer->serverName());
            m_localServer->listen(serverName);
        }
    }

#if defined(Q_WS_MAC)
    connect(this, SIGNAL(lastWindowClosed()),
            this, SLOT(lastWindowClosed()));
#endif

    // settings
    m_settings = new GtMainSettings(this);
    m_settings->load();

    // document loader
    m_docThread = new QThread(this);
    m_docLoader = new GtDocLoader(this);

    QDir dir(QCoreApplication::applicationDirPath());

    if (dir.cd("loader"))
        m_docLoader->registerLoaders(dir.absolutePath());

    m_docThread->start();

    // network
    m_userClient = new GtUserClient(this);

    QTimer::singleShot(0, this, SLOT(postLaunch()));
}

GtApplication::~GtApplication()
{
    m_docThread->quit();
    m_docThread->wait();

    for (int i = 0; i < m_mainWindows.size(); ++i) {
        GtMainWindow *window = m_mainWindows.at(i);
        delete window;
    }

    clearDocuments();
    Q_ASSERT(m_documents.size() == 0);

    m_settings->save();
}

bool GtApplication::isTheOnlyReader() const
{
    return (m_localServer != 0);
}

GtMainWindow* GtApplication::mainWindow()
{
    clearWindows();

    if (m_mainWindows.isEmpty())
        newMainWindow();

    return m_mainWindows[0];
}

QList<GtMainWindow*> GtApplication::mainWindows()
{
    clearWindows();

    QList<GtMainWindow*> list;
    for (int i = 0; i < m_mainWindows.count(); ++i)
        list.append(m_mainWindows.at(i));

    return list;
}

QThread* GtApplication::renderThread()
{
    return m_docThread;
}

GtDocumentPointer GtApplication::loadDocument(const QString &fileName)
{
    QHash<QString, GtDocumentPointer>::iterator it;

    it = m_documents.find(fileName);
    if (it != m_documents.end())
        return it.value();

    GtDocumentPointer doc;

    clearDocuments();
    doc = m_docLoader->loadDocument(fileName, m_docThread);
    if (doc)
        m_documents.insert(fileName, doc);

    return doc;
}

GtApplication* GtApplication::instance()
{
    return static_cast<GtApplication*>(QCoreApplication::instance());
}

#if defined(Q_WS_MAC)
bool GtApplication::event(QEvent *event)
{
    switch (event->type()) {
    case QEvent::ApplicationActivate:
        Q_ASSERT(0);
        break;

    case QEvent::FileOpen:
        Q_ASSERT(0);
        break;

    default:
        break;
    }

    return QApplication::event(event);
}
#endif

void GtApplication::clearWindows()
{
    // clear up any deleted main windows
    for (int i = m_mainWindows.count() - 1; i >= 0; --i) {
        if (m_mainWindows.at(i).isNull())
            m_mainWindows.removeAt(i);
    }
}

void GtApplication::clearDocuments()
{
    // clear up any unreferenced documents
    QHash<QString, GtDocumentPointer>::iterator it;

    for (it = m_documents.begin(); it != m_documents.end();) {
        if (it.value()->ref.load() < 2) {
            it = m_documents.erase(it);
        }
        else {
            ++it;
        }
    }
}

GtMainWindow* GtApplication::newMainWindow()
{
    GtMainWindow *reader = new GtMainWindow();
    m_mainWindows.prepend(reader);
    reader->show();
    return reader;
}

#if defined(Q_WS_MAC)
void GtApplication::lastWindowClosed()
{
    Q_ASSERT(0);
}
#endif

#if defined(Q_WS_MAC)
void GtApplication::quitReader()
{
    Q_ASSERT(0);
}
#endif

void GtApplication::postLaunch()
{
    setWindowIcon(QIcon(QLatin1String(":/images/logo.bmp")));
    newMainWindow();
}

void GtApplication::newLocalSocketConnection()
{
    QLocalSocket *socket = m_localServer->nextPendingConnection();
    if (!socket)
        return;

    socket->waitForReadyRead(1000);
    QTextStream stream(socket);
    QString url;

    stream >> url;
    if (!url.isEmpty()) {
        Q_ASSERT(0);
    }

    delete socket;
    mainWindow()->raise();
    mainWindow()->activateWindow();
}

GT_END_NAMESPACE
