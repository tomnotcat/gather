/*
 * Copyright (C) 2013 Tom Wong. All rights reserved.
 */
#include "gtclient.h"
#include "gtmessage.pb.h"
#include "gtrecvbuffer.h"
#include "gtsvcutil.h"
#include <QtCore/QDebug>
#include <QtNetwork/QTcpSocket>

GT_BEGIN_NAMESPACE

class GtClientPrivate
{
    Q_DECLARE_PUBLIC(GtClient)

public:
    explicit GtClientPrivate(GtClient *q);
    ~GtClientPrivate();

public:
    void sendLogin() const;

public:
    void handleMessage(const char *data, int size);
    void handleLogin(GtSimpleMessage &msg);

protected:
    GtClient *q_ptr;
    QTcpSocket *tcpSocket;
    GtRecvBuffer buffer;
    QString user;
    QString passwd;
    bool connected;
};

GtClientPrivate::GtClientPrivate(GtClient *q)
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

GtClientPrivate::~GtClientPrivate()
{
    if (connected) {
        tcpSocket->disconnectFromHost();
        if (tcpSocket->state() != QAbstractSocket::UnconnectedState)
            tcpSocket->waitForDisconnected();
    }
}

void GtClientPrivate::sendLogin() const
{
    GtLoginRequest msg;

    msg.set_user(user.toUtf8().constData());
    msg.set_passwd(passwd.toUtf8().constData());

    GtSvcUtil::sendMessage(tcpSocket, GT_LOGIN_REQUEST, msg);
}

void GtClientPrivate::handleMessage(const char *data, int size)
{
    Q_Q(GtClient);

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

void GtClientPrivate::handleLogin(GtSimpleMessage &msg)
{
    Q_Q(GtClient);

    emit q->onLogin(msg.data1());
}

GtClient::GtClient(QObject *parent)
    : QObject(parent)
    , d_ptr(new GtClientPrivate(this))
{
}

GtClient::~GtClient()
{
}

void GtClient::login(const QHostAddress &address, quint16 port,
                     const QString &user, const QString &passwd)
{
    Q_D(GtClient);

    d->user = user;
    d->passwd = passwd;

    if (d->connected) {
        d->sendLogin();
    }
    else {
        d->tcpSocket->connectToHost(address, port);
    }
}

void GtClient::logout()
{
    Q_D(GtClient);

    if (d->connected) {
        d->tcpSocket->disconnectFromHost();
        d->connected = false;
    }

    emit onLogout(LogoutNormal);
}

void GtClient::handleRead()
{
    Q_D(GtClient);

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

void GtClient::handleConnected()
{
    Q_D(GtClient);
    d->connected = true;
    d->sendLogin();
}

void GtClient::handleDisconnected()
{
    Q_D(GtClient);
    d->connected = false;
}

void GtClient::handleError(QAbstractSocket::SocketError error)
{
    Q_D(GtClient);

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
