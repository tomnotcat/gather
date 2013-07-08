/*
 * Copyright (C) 2013 Tom Wong. All rights reserved.
 */
#include "gtftclient.h"
#include "gtftfile.h"
#include "gtftmessage.pb.h"
#include "gtrecvbuffer.h"
#include "gtsvcutil.h"
#include <QtCore/QDebug>
#include <QtCore/qendian.h>
#include <QtCore/QMutex>
#include <QtNetwork/QHostAddress>
#include <QtNetwork/QTcpSocket>

GT_BEGIN_NAMESPACE

class GtFTClientPrivate : public GtFTFileHost
{
    Q_DECLARE_PUBLIC(GtFTClient)

public:
    explicit GtFTClientPrivate(GtFTClient *q);
    ~GtFTClientPrivate();

public:
    void close(bool wait);

public:
    void sendLogin() const;

public:
    void handleMessage(const char *data, int size);
    void handleLogin(GtFTLoginResponse &msg);

public:
    void closeFile(GtFTFile *file);
    QAbstractSocket* fileSocket();

protected:
    GtFTClient *q_ptr;
    QTcpSocket *socket;
    QList<GtFTFile*> files;
    GtRecvBuffer buffer;
    QMutex mutex;
    QHostAddress address;
    QString session;
    QString secret;
    quint16 port;
    bool connected;
    bool logined;
};

GtFTClientPrivate::GtFTClientPrivate(GtFTClient *q)
    : q_ptr(q)
    , port(0)
    , connected(false)
    , logined(false)
{
    socket = new QTcpSocket(q);
    q->connect(socket, SIGNAL(readyRead()), q, SLOT(handleRead()));
    q->connect(socket, SIGNAL(error(QAbstractSocket::SocketError)),
               q, SLOT(handleError(QAbstractSocket::SocketError)));
    q->connect(socket, SIGNAL(connected()), q, SLOT(handleConnected()));
    q->connect(socket, SIGNAL(disconnected()), q, SLOT(handleDisconnected()));
}

GtFTClientPrivate::~GtFTClientPrivate()
{
    close(true);
    qDeleteAll(files);
}

void GtFTClientPrivate::close(bool wait)
{
    if (!connected)
        return;

    socket->disconnectFromHost();
    if (wait) {
        if (socket->state() != QAbstractSocket::UnconnectedState)
            socket->waitForDisconnected();
    }

    connected = false;
    logined = false;
}

void GtFTClientPrivate::sendLogin() const
{
    GtFTLoginRequest msg;
    msg.set_session(session.toUtf8().constData());
    GtSvcUtil::sendMessage(socket, GT_FT_LOGIN_REQUEST, &msg);
}

void GtFTClientPrivate::handleMessage(const char *data, int size)
{
    if (size < (int)sizeof(quint16)) {
        qWarning() << "Invalid FT message size:" << size;
        return;
    }

    quint16 type = qFromBigEndian<quint16>(*(quint16*)data);
    data += sizeof(quint16);
    size -= sizeof(quint16);

    switch (type) {
    case GT_FT_LOGIN_RESPONSE:
        {
            GtFTLoginResponse msg;
            if (msg.ParseFromArray(data, size)) {
                handleLogin(msg);
            }
            else {
                qWarning() << "Invalid FT login response";
            }
        }
        break;

    default:
        qWarning() << "Invalid FT message type:" << type;
        break;
    }
}

void GtFTClientPrivate::handleLogin(GtFTLoginResponse &msg)
{
    Q_Q(GtFTClient);

    logined = (GtFTClient::LoginSuccess == msg.result());

    emit q->login(msg.result());
}

void GtFTClientPrivate::closeFile(GtFTFile *file)
{
    files.removeOne(file);
    delete file;
}

QAbstractSocket* GtFTClientPrivate::fileSocket()
{
    return socket;
}

GtFTClient::GtFTClient(QObject *parent)
    : QObject(parent)
    , d_ptr(new GtFTClientPrivate(this))
{
}

GtFTClient::~GtFTClient()
{
}

void GtFTClient::login(const QHostAddress &address, quint16 port,
                       const QString &session, const QString &secret)
{
    Q_D(GtFTClient);

    if (1) {
        QMutexLocker locker(&d->mutex);
        d->address = address;
        d->port = port;
        d->session = session;
        d->secret = secret;
    }

    QMetaObject::invokeMethod(this, "realLogin");
}

void GtFTClient::logout()
{
    QMetaObject::invokeMethod(this, "realLogout");
}

GtFTFile* GtFTClient::openFile(const QString &fileId)
{
    Q_D(GtFTClient);

    if (!d->logined)
        return 0;

    GtFTFile *file = new GtFTFile(d, fileId);
    d->files.push_back(file);

    return file;
}

void GtFTClient::realLogin()
{
    Q_D(GtFTClient);

    if (d->connected) {
        d->socket->disconnectFromHost();
        if (d->socket->state() != QAbstractSocket::UnconnectedState)
            d->socket->waitForDisconnected();
    }

    d->socket->connectToHost(d->address, d->port);
}

void GtFTClient::realLogout()
{
    Q_D(GtFTClient);

    d->close(false);

    emit logout(LogoutNormal);
}

void GtFTClient::handleRead()
{
    Q_D(GtFTClient);

    int result = d->buffer.read(d->socket);
    while (GtRecvBuffer::ReadMessage == result) {
        d->handleMessage(d->buffer.buffer(), d->buffer.size());
        d->buffer.clear();
        result = d->buffer.read(d->socket);
    }

    switch (result) {
    case GtRecvBuffer::ReadError:
        qWarning() << "GtFTClient GtRecvBuffer::ReadError";
        break;

    default:
        break;
    }
}

void GtFTClient::handleConnected()
{
    Q_D(GtFTClient);
    d->connected = true;
    d->sendLogin();
}

void GtFTClient::handleDisconnected()
{
    Q_D(GtFTClient);
    d->connected = false;
    d->logined = false;
}

void GtFTClient::handleError(QAbstractSocket::SocketError error)
{
    Q_D(GtFTClient);

    d->connected = false;
    d->logined = false;

    switch (error) {
    case QAbstractSocket::RemoteHostClosedError:
        break;

    default:
        qWarning() << "GtFTClient socket error:"
                   << error << d->socket->errorString();
        break;
    }
}

GT_END_NAMESPACE
