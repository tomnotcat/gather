/*
 * Copyright (C) 2013 Tom Wong. All rights reserved.
 */
#include "gtuserserver.h"
#include "gtusersession.h"
#include <QtCore/QDebug>
#include <QtCore/QMutex>

GT_BEGIN_NAMESPACE

class GtUserServerPrivate
{
    Q_DECLARE_PUBLIC(GtUserServer)

public:
    GtUserServerPrivate(GtUserServer *q);
    ~GtUserServerPrivate();

protected:
    GtUserServer *q_ptr;
    QHash<QString, GtUserSession*> users;
    QMutex mutex;
};

GtUserServerPrivate::GtUserServerPrivate(GtUserServer *q)
    : q_ptr(q)
{
}

GtUserServerPrivate::~GtUserServerPrivate()
{
}

GtUserServer::GtUserServer(QObject *parent)
    : GtServer(parent)
    , d_ptr(new GtUserServerPrivate(this))
{
}

GtUserServer::~GtUserServer()
{
}

void GtUserServer::addLogin(GtUserSession *session)
{
    Q_D(GtUserServer);

    QString key = session->name();
    Q_ASSERT(!key.isEmpty());

    QMutexLocker locker(&d->mutex);

    QHash<QString, GtUserSession*>::iterator it = d->users.find(key);
    if (it != d->users.end()) {
        QMetaObject::invokeMethod(*it, "reloginLogout", Qt::QueuedConnection);
        d->users.erase(it);
    }

    d->users.insert(key, session);
}

GtSession* GtUserServer::createSession()
{
    return new GtUserSession();
}

void GtUserServer::removeSession(GtSession *session)
{
    Q_D(GtUserServer);

    GtUserSession *user = qobject_cast<GtUserSession*>(session);
    QMutexLocker locker(&d->mutex);
    d->users.remove(user->name());
}

GT_END_NAMESPACE
