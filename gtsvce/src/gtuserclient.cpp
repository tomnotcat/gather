/*
 * Copyright (C) 2013 Tom Wong. All rights reserved.
 */
#include "gtuserclient.h"
#include "gtmessage.pb.h"
#include "gtrecvbuffer.h"
#include "gtsvcutil.h"
#include <QtCore/QDebug>
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
    void handleLogin(GtSimpleMessage &msg);

protected:
    GtUserClient *q_ptr;
    QTcpSocket *tcpSocket;
    GtRecvBuffer buffer;
    QString user;
    QString passwd;
    bool connected;
};

GtUserClientPrivate::GtUserClientPrivate(GtUserClient *q)
    : q_ptr(q)
    , connected(false)
{
    tcpSocket = new QTcpSocket(q);
    q->connect(tcpSocket, SIGNAL(readyRead()), q, SLOT(handleRead()));
    q->connect(tcpSocket, SIGNAL(error(QAbstractSocket::SocketError)),
               q, SLOT(handleError(QAbstractSocket::SocketError)));
    q->connect(tcpSocket, SIGNAL(connected()), q, SLOT(handleConnected()));
    q->connect(tcpSocket, SIGNAL(disconnected()), q, SLOT(handleDisconnected()));
}

GtUserClientPrivate::~GtUserClientPrivate()
{
    if (connected) {
        tcpSocket->disconnectFromHost();
        if (tcpSocket->state() != QAbstractSocket::UnconnectedState)
            tcpSocket->waitForDisconnected();
    }
}

void GtUserClientPrivate::sendLogin() const
{
    GtLoginRequest msg;

    msg.set_user(user.toUtf8().constData());
    msg.set_passwd(passwd.toUtf8().constData());

    GtSvcUtil::sendMessage(tcpSocket, GT_LOGIN_REQUEST, msg);
}

void GtUserClientPrivate::handleMessage(const char *data, int size)
{
    Q_Q(GtUserClient);

    if (size < (int)sizeof(quint16)) {
        qWarning() << "Invalid message size:" << size;
        return;
    }

    quint16 type = ntohs(*(quint16*)data);
    data += sizeof(quint16);
    size -= sizeof(quint16);

    switch (type) {
    case GT_LOGIN_RESPONSE:
        {
            GtSimpleMessage msg;
            if (msg.ParseFromArray(data, size)) {
                handleLogin(msg);
            }
            else {
                qWarning() << "Invalid login response";
            }
        }
        break;

    case GT_LOGOUT_MESSAGE:
        {
            GtSimpleMessage msg;
            if (msg.ParseFromArray(data, size)) {
                emit q->onLogout(msg.data1());
            }
            else {
                qWarning() << "Invalid logout response";
            }
        }
        break;

    default:
        qWarning() << "Invalid message type:" << type;
        break;
    }
}

void GtUserClientPrivate::handleLogin(GtSimpleMessage &msg)
{
    Q_Q(GtUserClient);

    emit q->onLogin(msg.data1());
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
        d->tcpSocket->connectToHost(address, port);
    }
}

void GtUserClient::logout()
{
    Q_D(GtUserClient);

    if (d->connected) {
        d->tcpSocket->disconnectFromHost();
        d->connected = false;
    }

    emit onLogout(LogoutNormal);
}

void GtUserClient::handleRead()
{
    Q_D(GtUserClient);

    int result = d->buffer.read(d->tcpSocket);
    while (GtRecvBuffer::ReadMessage == result) {
        d->handleMessage(d->buffer.buffer(), d->buffer.size());
        d->buffer.clear();
        result = d->buffer.read(d->tcpSocket);
    }

    switch (result) {
    case GtRecvBuffer::ReadError:
        qWarning() << "GtRecvBuffer::ReadError";
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
}

void GtUserClient::handleError(QAbstractSocket::SocketError error)
{
    Q_D(GtUserClient);

    d->connected = false;

    switch (error) {
    case QAbstractSocket::RemoteHostClosedError:
        break;

    default:
        qWarning() << "GtClient socket error:"
                   << error << d->tcpSocket->errorString();
        break;
    }
}

GT_END_NAMESPACE
