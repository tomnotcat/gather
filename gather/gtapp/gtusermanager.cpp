/*
 * Copyright (C) 2013 Tom Wong. All rights reserved.
 */
#include "gtusermanager_p.h"
#include "gtuserclient.h"
#include <QtCore/QDebug>
#include <QtCore/QMutex>
#include <QtNetwork/QHostAddress>

GT_BEGIN_NAMESPACE

class GtUserManagerPrivate
{
    Q_DECLARE_PUBLIC(GtUserManager)

public:
    GtUserManagerPrivate(GtUserManager *q, QThread *t);
    ~GtUserManagerPrivate();

public:
    GtUserManager *q_ptr;
    GtUserManagerProxy *m_proxy;
    QThread *m_thread;
    GtUserClient *m_client;
    QString m_session;
    int m_state;
    QMutex m_mutex;
};

GtUserManagerProxy::GtUserManagerProxy(GtUserManagerPrivate *p)
    : m_priv(p)
{
}

GtUserManagerProxy::~GtUserManagerProxy()
{
}

void GtUserManagerProxy::connect(const QString &host, quint16 port)
{
    GtUserClient *client = m_priv->m_client;

    if (client->connect(QHostAddress(host), port))
        return;

    emit m_priv->q_ptr->connectError(client->error());
    client->clearError();
}

void GtUserManagerProxy::disconnect()
{
    m_priv->m_client->disconnect();
}

void GtUserManagerProxy::login(const QString &user, const QString &passwd)
{
    GtUserClient *client = m_priv->m_client;

    if (client->login(user, passwd))
        return;

    emit m_priv->q_ptr->loginError(client->error());
    client->clearError();
}

void GtUserManagerProxy::logout()
{
    GtUserClient *client = m_priv->m_client;
    client->logout();
}

void GtUserManagerProxy::stateChanged(int state)
{
    GtUserClient *client = m_priv->m_client;

    m_priv->m_mutex.lock();
    m_priv->m_state = state;
    m_priv->m_session = client->session();
    m_priv->m_mutex.unlock();

    emit m_priv->q_ptr->stateChanged(state, client->error());
    client->clearError();
}

GtUserManagerPrivate::GtUserManagerPrivate(GtUserManager *q, QThread *t)
    : q_ptr(q)
    , m_thread(t)
    , m_client(0)
    , m_state(GtUserClient::UnconnectedState)
{
    m_client = new GtUserClient();
    m_proxy = new GtUserManagerProxy(this);

    QObject::connect(m_client, SIGNAL(stateChanged(int)),
                     m_proxy, SLOT(stateChanged(int)));

    if (t) {
        m_client->moveToThread(t);
        m_proxy->moveToThread(t);
    }
}

GtUserManagerPrivate::~GtUserManagerPrivate()
{
    delete m_proxy;
    delete m_client;
}

GtUserManager::GtUserManager(QThread *thread, QObject *parent)
    : QObject(parent)
    , d_ptr(new GtUserManagerPrivate(this, thread))
{
}

GtUserManager::~GtUserManager()
{
}

void GtUserManager::connect(const QString &host, quint16 port)
{
    Q_D(GtUserManager);

    QMetaObject::invokeMethod(d->m_proxy, "connect",
                              Q_ARG(QString, host),
                              Q_ARG(quint16, port));
}

void GtUserManager::disconnect()
{
    Q_D(GtUserManager);

    QMetaObject::invokeMethod(d->m_proxy, "disconnect");
}

void GtUserManager::login(const QString &user, const QString &passwd)
{
    Q_D(GtUserManager);

    QMetaObject::invokeMethod(d->m_proxy, "login",
                              Q_ARG(QString, user),
                              Q_ARG(QString, passwd));
}

void GtUserManager::logout()
{
    Q_D(GtUserManager);

    QMetaObject::invokeMethod(d->m_proxy, "logout");
}

QString GtUserManager::session() const
{
    QMutexLocker locker(&d_ptr->m_mutex);
    QString session(d_ptr->m_session);
    return session;
}

int GtUserManager::state() const
{
    QMutexLocker locker(&d_ptr->m_mutex);
    int state = d_ptr->m_state;
    return state;
}

GT_END_NAMESPACE
