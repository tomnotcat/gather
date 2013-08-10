/*
 * Copyright (C) 2013 Tom Wong. All rights reserved.
 */
#include "gtsession_p.h"
#include "gtserver_p.h"
#include <QtCore/QDebug>
#include <QtNetwork/QAbstractSocket>

GT_BEGIN_NAMESPACE

GtSessionPrivate::GtSessionPrivate(GtSession *q)
    : q_ptr(q)
    , socket(0)
    , server(0)
    , thread(0)
{
}

GtSessionPrivate::~GtSessionPrivate()
{
    delete socket;
}

void GtSessionPrivate::init(QAbstractSocket *s, GtServer *m, GtServerThread *t)
{
    Q_Q(GtSession);

    socket = s;
    server = m;
    thread = t;

    s->connect(s, SIGNAL(readyRead()), q, SLOT(handleRead()));
    s->connect(s, SIGNAL(error(QAbstractSocket::SocketError)),
               q, SLOT(handleError(QAbstractSocket::SocketError)));
}

GtSession::GtSession(QObject *parent)
    : QObject(parent)
    , d_ptr(new GtSessionPrivate(this))
{
}

GtSession::~GtSession()
{
}

void GtSession::message(const char *data, int size)
{
    Q_UNUSED(data);
    Q_UNUSED(size);
}

GtServer* GtSession::server() const
{
    Q_D(const GtSession);
    return d->server;
}

QAbstractSocket* GtSession::socket() const
{
    Q_D(const GtSession);
    return d->socket;
}

void GtSession::close()
{
    Q_D(GtSession);

    d->socket->close();
    d->server->d_ptr->removeSession(this);
    deleteLater();
}

void GtSession::handleRead()
{
    Q_D(GtSession);

    int result = d->buffer.read(d->socket, false);
    while (GtRecvBuffer::ReadMessage == result) {
        this->message(d->buffer.buffer(), d->buffer.size());
        d->buffer.clear();
        result = d->buffer.read(d->socket, false);
    }

    switch (result) {
    case GtRecvBuffer::ReadError:
        qWarning() << "GtRecvBuffer::ReadError";
        close();
        break;

    default:
        break;
    }
}

void GtSession::handleError(QAbstractSocket::SocketError error)
{
    Q_D(GtSession);

    switch (error) {
    case QAbstractSocket::RemoteHostClosedError:
        break;

    default:
        qWarning() << "GtSession socket error:"
                   << error << d->socket->errorString();
        break;
    }

    this->close();
}

GT_END_NAMESPACE
