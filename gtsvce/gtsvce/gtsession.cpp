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
    , m_socket(0)
    , m_server(0)
    , m_thread(0)
    , m_peerPort(0)
{
}

GtSessionPrivate::~GtSessionPrivate()
{
    delete m_socket;
}

void GtSessionPrivate::init(QAbstractSocket *s, GtServer *m, GtServerThread *t)
{
    Q_Q(GtSession);

    m_socket = s;
    m_server = m;
    m_thread = t;
    m_peerAddress = s->peerAddress();
    m_peerPort = s->peerPort();
    m_peerName = m_peerAddress.toString() + ":" + QString::number(m_peerPort);

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
    return d->m_server;
}

QAbstractSocket* GtSession::socket() const
{
    Q_D(const GtSession);
    return d->m_socket;
}

void GtSession::close()
{
    Q_D(GtSession);

    d->m_socket->close();
    d->m_server->d_ptr->removeSession(this);
    deleteLater();
}

QHostAddress GtSession::peerAddress() const
{
    Q_D(const GtSession);
    return d->m_peerAddress;
}

quint16 GtSession::peerPort() const
{
    Q_D(const GtSession);
    return d->m_peerPort;
}

QString GtSession::peerName() const
{
    Q_D(const GtSession);
    return d->m_peerName;
}

void GtSession::handleRead()
{
    Q_D(GtSession);

    int result = d->m_buffer.read(d->m_socket, false);
    while (GtRecvBuffer::ReadMessage == result) {
        message(d->m_buffer.buffer(), d->m_buffer.size());
        d->m_buffer.clear();
        result = d->m_buffer.read(d->m_socket, false);
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
                   << error << d->m_socket->errorString();
        break;
    }

    close();
}

GT_END_NAMESPACE
