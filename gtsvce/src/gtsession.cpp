/*
 * Copyright (C) 2013 Tom Wong. All rights reserved.
 */
#include "gtsession_p.h"
#include <QtCore/QDebug>
#include <QtNetwork/QAbstractSocket>

GT_BEGIN_NAMESPACE

GtSessionPrivate::GtSessionPrivate(GtSession *q)
    : q_ptr(q)
    , socket(0)
{
}

GtSessionPrivate::~GtSessionPrivate()
{
    delete socket;
}

void GtSessionPrivate::setSocket(QAbstractSocket *s)
{
    Q_Q(GtSession);

    if (socket)
        socket->deleteLater();

    socket = s;

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

QAbstractSocket* GtSession::socket() const
{
    Q_D(const GtSession);
    return d->socket;
}

void GtSession::close()
{
}

void GtSession::handleRead()
{
    Q_D(GtSession);

    int result = d->buffer.read(d->socket);
    while (GtRecvBuffer::ReadMessage == result) {
        this->message(d->buffer.buffer(), d->buffer.size());
        d->buffer.clear();
        result = d->buffer.read(d->socket);
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
}

GT_END_NAMESPACE
