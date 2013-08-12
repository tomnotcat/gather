/*
 * Copyright (C) 2013 Tom Wong. All rights reserved.
 */
#include "gtserver_p.h"
#include "gtsession_p.h"
#include <QtCore/QDebug>
#include <QtNetwork/QTcpSocket>

GT_BEGIN_NAMESPACE

GtServerPrivate::GtServerPrivate(GtServer *q)
    : q_ptr(q)
    , maxThread(0)
    , closing(false)
{
}

GtServerPrivate::~GtServerPrivate()
{
    close();
}

GtServerThread* GtServerPrivate::fetchThread()
{
    GtServerThread *thread = 0;

    if (threads.size() < maxThread || threads.size() == 0) {
        thread = new GtServerThread();
        thread->start();
        threads.push_back(thread);
        return thread;
    }

    int lowest = -1;
    foreach(GtServerThread *t, threads) {
        if (-1 == lowest || t->sessions.size() < lowest) {
            thread = t;
            lowest = t->sessions.size();
        }
    }

    return thread;
}

void GtServerPrivate::addSession(GtSession *session, qintptr socketDescriptor)
{
    Q_Q(GtServer);

    QMutexLocker locker(&mutex);
    GtServerThread *thread = fetchThread();
    QTcpSocket *socket = new QTcpSocket(session);

    socket->setSocketDescriptor(socketDescriptor);
    session->d_ptr->init(socket, q, thread);

    qDebug() << "Add session:" << session->peerAddress().toString();

    thread->sessions.push_back(session);
    session->moveToThread(thread);

    q->addSession(session);
}

void GtServerPrivate::removeSession(GtSession *session)
{
    Q_Q(GtServer);

    GtServerThread *thread = session->d_ptr->m_thread;

    Q_ASSERT(session->thread() == thread);

    QMutexLocker locker(&mutex);
    thread->sessions.removeOne(session);

    if (!closing) {
        QHostAddress address(session->peerAddress());

        q->removeSession(session);
        qDebug() << "Remove session:" << address.toString();
    }
}

void GtServerPrivate::close()
{
    closing = true;

    foreach(GtServerThread *t, threads) {
        t->quit();
        t->wait();
        qDeleteAll(t->sessions);
    }

    qDeleteAll(threads);
    threads.clear();
    closing = false;
}

GtServer::GtServer(QObject *parent)
    : QTcpServer(parent)
    , d_ptr(new GtServerPrivate(this))
{
}

GtServer::~GtServer()
{
}

void GtServer::setMaxThread(int count)
{
    Q_D(GtServer);
    d->maxThread = count;
}

int GtServer::maxThread() const
{
    Q_D(const GtServer);
    return d->maxThread;
}

void GtServer::close()
{
    Q_D(GtServer);
    d->close();
    QTcpServer::close();
}

void GtServer::addSession(GtSession *session)
{
    Q_UNUSED(session);
}

void GtServer::removeSession(GtSession *session)
{
    Q_UNUSED(session);
}

void GtServer::incomingConnection(qintptr socketDescriptor)
{
    Q_D(GtServer);
    d->addSession(createSession(), socketDescriptor);
}

GT_END_NAMESPACE
