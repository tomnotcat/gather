/*
 * Copyright (C) 2013 Tom Wong. All rights reserved.
 */
#include "gtclient.h"
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
    void sendLogin();

protected:
    GtClient *q_ptr;
    QTcpSocket *tcpSocket;
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
}

GtClientPrivate::~GtClientPrivate()
{
}

void GtClientPrivate::sendLogin()
{
    qDebug() << "login:" << user << passwd;
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

void GtClient::handleRead()
{
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
    qWarning() << "socket error:" << error << d->tcpSocket->errorString();
}

GT_END_NAMESPACE
