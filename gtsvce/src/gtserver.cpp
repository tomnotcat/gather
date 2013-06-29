/*
 * Copyright (C) 2013 Tom Wong. All rights reserved.
 */
#include "gtserver.h"
#include <QtCore/QDebug>
#include <QtNetwork/QTcpServer>
#include <QtNetwork/QTcpSocket>

GT_BEGIN_NAMESPACE

class GtServerPrivate
{
    Q_DECLARE_PUBLIC(GtServer)

public:
    explicit GtServerPrivate(GtServer *q);
    ~GtServerPrivate();

protected:
    GtServer *q_ptr;
    QTcpServer *tcpServer;
};

GtServerPrivate::GtServerPrivate(GtServer *q)
    : q_ptr(q)
{
    tcpServer = new QTcpServer(q);
    q->connect(tcpServer, SIGNAL(newConnection()), q, SLOT(handleNewConnection()));
}

GtServerPrivate::~GtServerPrivate()
{
}

GtServer::GtServer(QObject *parent)
    : QObject(parent)
    , d_ptr(new GtServerPrivate(this))
{
}

GtServer::~GtServer()
{
}

bool GtServer::listen(quint16 port, const QHostAddress &address)
{
    Q_D(GtServer);
    return d->tcpServer->listen(address, port);
}

void GtServer::handleNewConnection()
{
    Q_D(GtServer);

    QTcpSocket *connection = d->tcpServer->nextPendingConnection();
    connection->deleteLater();
    qDebug() << "new connection";
}

GT_END_NAMESPACE
