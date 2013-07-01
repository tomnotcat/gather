/*
 * Copyright (C) 2013 Tom Wong. All rights reserved.
 */
#include "gtsessionmanager.h"
#include "gtsession_p.h"
#include "gtusersession.h"
#include <QtCore/QDebug>
#include <QtCore/QThread>
#include <QtNetwork/QTcpSocket>

GT_BEGIN_NAMESPACE

class GtSessionManagerPrivate
{
    Q_DECLARE_PUBLIC(GtSessionManager)

public:
    struct Thread
    {
        QThread thread;
        QList<GtSession*> sessions;
    };

public:
    GtSessionManagerPrivate(GtSessionManager *q);
    ~GtSessionManagerPrivate();

public:
    Thread* fetchThread();

protected:
    GtSessionManager *q_ptr;
    QList<Thread*> threads;
    int maxThread;
};

GtSessionManagerPrivate::GtSessionManagerPrivate(GtSessionManager *q)
    : q_ptr(q)
    , maxThread(0)
{
}

GtSessionManagerPrivate::~GtSessionManagerPrivate()
{
    foreach(Thread *t, threads) {
        t->thread.quit();
        t->thread.wait();
        qDeleteAll(t->sessions);
    }

    qDeleteAll(threads);
}

GtSessionManagerPrivate::Thread* GtSessionManagerPrivate::fetchThread()
{
    Thread *thread = 0;

    if (threads.size() < maxThread || threads.size() == 0) {
        thread = new Thread();
        thread->thread.start();
        return thread;
    }

    int lowest = -1;
    foreach(Thread *t, threads) {
        if (-1 == lowest || t->sessions.size() < lowest) {
            thread = t;
            lowest = t->sessions.size();
        }
    }

    return thread;
}

GtSessionManager::GtSessionManager(QObject *parent)
    : QObject(parent)
    , d_ptr(new GtSessionManagerPrivate(this))
{
}

GtSessionManager::~GtSessionManager()
{
}

void GtSessionManager::setMaxThread(int count)
{
    Q_D(GtSessionManager);
    d->maxThread = count;
}

void GtSessionManager::addSession(qintptr socketDescriptor)
{
    Q_D(GtSessionManager);

    GtSessionManagerPrivate::Thread *thread = d->fetchThread();
    GtSession *session = new GtUserSession();
    QTcpSocket *socket = new QTcpSocket(session);
    socket->setSocketDescriptor(socketDescriptor);
    session->d_ptr->socket = socket;

    thread->sessions.push_back(session);
    session->moveToThread(&thread->thread);
}

GT_END_NAMESPACE
