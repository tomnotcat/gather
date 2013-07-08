/*
 * Copyright (C) 2013 Tom Wong. All rights reserved.
 */
#include "gtuserclient.h"
#include "gtrecvbuffer.h"
#include "gtsvcutil.h"
#include "gtusermessage.pb.h"
#include <QtCore/QDebug>
#include <QtCore/qendian.h>
#include <QtNetwork/QTcpSocket>

GT_BEGIN_NAMESPACE

class GtUserClientPrivate
{
    Q_DECLARE_PUBLIC(GtUserClient)

public:
    explicit GtUserClientPrivate(GtUserClient *q);
    ~GtUserClientPrivate();

public:
    void sendLogin() const;

public:
    void handleMessage(const char *data, int size);
    void handleLogin(GtUserLoginResponse &msg);

protected:
    GtUserClient *q_ptr;
    QTcpSocket *socket;
    GtRecvBuffer buffer;
    QString user;
    QString passwd;
    bool connected;
    bool logined;
};

GtUserClientPrivate::GtUserClientPrivate(GtUserClient *q)
    : q_ptr(q)
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

GtUserClientPrivate::~GtUserClientPrivate()
{
    if (connected) {
        socket->disconnectFromHost();
        if (socket->state() != QAbstractSocket::UnconnectedState)
            socket->waitForDisconnected();
    }
}

void GtUserClientPrivate::sendLogin() const
{
    GtUserLoginRequest msg;

    msg.set_user(user.toUtf8().constData());
    msg.set_passwd(passwd.toUtf8().constData());

    GtSvcUtil::sendMessage(socket, GT_USER_LOGIN_REQUEST, &msg);
}

void GtUserClientPrivate::handleMessage(const char *data, int size)
{
    Q_Q(GtUserClient);

    if (size < (int)sizeof(quint16)) {
        qWarning() << "Invalid user message size:" << size;
        return;
    }

    quint16 type = qFromBigEndian<quint16>(*(quint16*)data);
    data += sizeof(quint16);
    size -= sizeof(quint16);

    switch (type) {
    case GT_USER_LOGIN_RESPONSE:
        {
            GtUserLoginResponse msg;
            if (msg.ParseFromArray(data, size)) {
                handleLogin(msg);
            }
            else {
                qWarning() << "Invalid user login response";
            }
        }
        break;

    case GT_USER_LOGOUT_RESPONSE:
        {
            GtUserLogoutResponse msg;
            if (msg.ParseFromArray(data, size)) {
                emit q->logout(msg.reason());
            }
            else {
                qWarning() << "Invalid user logout response";
            }
        }
        break;

    default:
        qWarning() << "Invalid user message type:" << type;
        break;
    }
}

void GtUserClientPrivate::handleLogin(GtUserLoginResponse &msg)
{
    Q_Q(GtUserClient);

    logined = (GtUserClient::LoginSuccess == msg.result());

    emit q->login(msg.result());
}

GtUserClient::GtUserClient(QObject *parent)
    : QObject(parent)
    , d_ptr(new GtUserClientPrivate(this))
{
}

GtUserClient::~GtUserClient()
{
}

void GtUserClient::login(const QHostAddress &address, quint16 port,
                         const QString &user, const QString &passwd)
{
    Q_D(GtUserClient);

    d->user = user;
    d->passwd = passwd;

    if (d->connected) {
        d->sendLogin();
    }
    else {
        d->socket->connectToHost(address, port);
    }
}

void GtUserClient::logout()
{
    Q_D(GtUserClient);

    if (d->connected) {
        d->socket->disconnectFromHost();
        d->connected = false;
        d->logined = false;
    }

    emit logout(LogoutNormal);
}

void GtUserClient::handleRead()
{
    Q_D(GtUserClient);

    int result = d->buffer.read(d->socket);
    while (GtRecvBuffer::ReadMessage == result) {
        d->handleMessage(d->buffer.buffer(), d->buffer.size());
        d->buffer.clear();
        result = d->buffer.read(d->socket);
    }

    switch (result) {
    case GtRecvBuffer::ReadError:
        qWarning() << "GtUserClient GtRecvBuffer::ReadError";
        break;

    default:
        break;
    }
}

void GtUserClient::handleConnected()
{
    Q_D(GtUserClient);
    d->connected = true;
    d->sendLogin();
}

void GtUserClient::handleDisconnected()
{
    Q_D(GtUserClient);
    d->connected = false;
    d->logined = false;
}

void GtUserClient::handleError(QAbstractSocket::SocketError error)
{
    Q_D(GtUserClient);

    d->connected = false;
    d->logined = false;

    switch (error) {
    case QAbstractSocket::RemoteHostClosedError:
        break;

    default:
        qWarning() << "GtUserClient socket error:"
                   << error << d->socket->errorString();
        break;
    }
}

GT_END_NAMESPACE
