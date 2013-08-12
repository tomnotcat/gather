/*
 * Copyright (C) 2013 Tom Wong. All rights reserved.
 */
#include "gtapplication.h"
#include "gtdocmanager.h"
#include "gtmainsettings.h"
#include "gtmainwindow.h"
#include "gtusermanager.h"
#include <QtCore/QCryptographicHash>
#include <QtCore/QDebug>
#include <QtCore/QDir>
#include <QtCore/QPointer>
#include <QtCore/QStandardPaths>
#include <QtCore/QStringList>
#include <QtCore/QTextStream>
#include <QtCore/QThread>
#include <QtCore/QTimer>
#include <QtGui/QFileOpenEvent>
#include <QtGui/QIcon>
#include <QtNetwork/QHostAddress>
#include <QtNetwork/QLocalServer>
#include <QtNetwork/QLocalSocket>

GT_BEGIN_NAMESPACE

class GtApplicationPrivate
{
    Q_DECLARE_PUBLIC(GtApplication)

public:
    GtApplicationPrivate(GtApplication *q);
    ~GtApplicationPrivate();

public:
    void clearWindows();

protected:
    GtApplication *q_ptr;

    QList<QPointer<GtMainWindow> > m_mainWindows;
    QLocalServer *m_localServer;

    GtMainSettings *m_settings;

    // document
    QThread *m_docThread;
    GtDocManager *m_docManager;

    // network
    QThread *m_networkThread;
    GtUserManager *m_userManager;
};

GtApplicationPrivate::GtApplicationPrivate(GtApplication *q)
    : q_ptr(q)
    , m_localServer(0)
    , m_docThread(0)
    , m_docManager(0)
    , m_networkThread(0)
    , m_userManager(0)
{
    QCoreApplication::setOrganizationName(QLatin1String("Clue"));
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

    m_localServer = new QLocalServer(q);
    q->connect(m_localServer, SIGNAL(newConnection()),
               q, SLOT(newLocalSocketConnection()));

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
    m_settings = new GtMainSettings(q);
    m_settings->load();

    QTimer::singleShot(0, q, SLOT(postLaunch()));
}

GtApplicationPrivate::~GtApplicationPrivate()
{
    if (m_docThread) {
        m_docThread->quit();
        m_docThread->wait();
    }

    for (int i = 0; i < m_mainWindows.size(); ++i) {
        GtMainWindow *window = m_mainWindows.at(i);
        delete window;
    }

    m_settings->save();

    if (m_networkThread) {
        m_networkThread->quit();
        m_networkThread->wait();
    }
}

void GtApplicationPrivate::clearWindows()
{
    // clear up any deleted main windows
    for (int i = m_mainWindows.count() - 1; i >= 0; --i) {
        if (m_mainWindows.at(i).isNull())
            m_mainWindows.removeAt(i);
    }
}

GtApplication::GtApplication(int &argc, char **argv)
    : QApplication(argc, argv)
    , d_ptr(new GtApplicationPrivate(this))
{
    setWindowIcon(QIcon(QLatin1String(":/images/logo.bmp")));

    // remove frame around status bar items
    setStyleSheet("QStatusBar::item { border: 0px }");
}

GtApplication::~GtApplication()
{
}

bool GtApplication::isTheOnlyReader() const
{
    Q_D(const GtApplication);
    return (d->m_localServer != 0);
}

GtMainWindow *GtApplication::mainWindow()
{
    Q_D(GtApplication);

    d->clearWindows();

    if (d->m_mainWindows.isEmpty())
        newMainWindow();

    return d->m_mainWindows[0];
}

QList<GtMainWindow*> GtApplication::mainWindows()
{
    Q_D(GtApplication);

    d->clearWindows();

    QList<GtMainWindow*> list;
    for (int i = 0; i < d->m_mainWindows.count(); ++i)
        list.append(d->m_mainWindows.at(i));

    return list;
}

GtMainSettings *GtApplication::settings()
{
    Q_D(GtApplication);
    return d->m_settings;
}

QThread *GtApplication::docThread()
{
    Q_D(GtApplication);

    if (!d->m_docThread) {
        d->m_docThread = new QThread(this);
        d->m_docThread->start();
    }

    return d->m_docThread;
}

GtDocManager *GtApplication::docManager()
{
    Q_D(GtApplication);

    if (!d->m_docManager) {
        QString docdb = dataFilePath("document.db");
        QThread *thread = docThread();
        d->m_docManager = new GtDocManager(docdb, thread, this);

        QDir dir(QCoreApplication::applicationDirPath());
        if (dir.cd("loader"))
            d->m_docManager->registerLoaders(dir.absolutePath());
        else
            qWarning() << "can't access loaders directory";
    }

    return d->m_docManager;
}

QThread *GtApplication::networkThread()
{
    Q_D(GtApplication);

    if (!d->m_networkThread) {
        d->m_networkThread = new QThread(this);
        d->m_networkThread->start();
    }

    return d->m_networkThread;
}

GtUserManager *GtApplication::userManager()
{
    Q_D(GtApplication);

    if (!d->m_userManager) {
        QString regdb = dataFilePath("registry.db");
        QThread *thread = networkThread();
        d->m_userManager = new GtUserManager(regdb, thread, this);
    }

    return d->m_userManager;
}

GtApplication* GtApplication::instance()
{
    return static_cast<GtApplication*>(QCoreApplication::instance());
}

QString GtApplication::dataFilePath(const QString &fileName)
{
    QDir dir(QStandardPaths::writableLocation(QStandardPaths::DataLocation));
    dir.mkpath(".");

    return dir.absoluteFilePath(fileName);
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

GtMainWindow* GtApplication::newMainWindow()
{
    Q_D(GtApplication);

    GtMainWindow *window = new GtMainWindow();
    d->m_mainWindows.prepend(window);
    window->show();
    return window;
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
    // connect to server
    userManager();

    // new default window
    newMainWindow();
}

void GtApplication::newLocalSocketConnection()
{
    Q_D(GtApplication);

    QLocalSocket *socket = d->m_localServer->nextPendingConnection();
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
