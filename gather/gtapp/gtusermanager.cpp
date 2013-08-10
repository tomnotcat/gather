/*
 * Copyright (C) 2013 Tom Wong. All rights reserved.
 */
#include "gtusermanager_p.h"
#include "gtuserclient.h"
#include <QtCore/QDebug>

GT_BEGIN_NAMESPACE

class GtUserManagerPrivate
{
    Q_DECLARE_PUBLIC(GtUserManager)

public:
    GtUserManagerPrivate(GtUserManager *q, QThread *t);
    ~GtUserManagerPrivate();

protected:
    GtUserManager *q_ptr;
    QThread *m_networkThread;
    GtUserClient *m_userClient;
};

GtUserManagerProxy::GtUserManagerProxy()
{
}

GtUserManagerProxy::~GtUserManagerProxy()
{
}

GtUserManagerPrivate::GtUserManagerPrivate(GtUserManager *q, QThread *t)
    : q_ptr(q)
    , m_networkThread(t)
{
    m_userClient = new GtUserClient();
    m_userClient->moveToThread(t);
}

GtUserManagerPrivate::~GtUserManagerPrivate()
{
    delete m_userClient;
}

GtUserManager::GtUserManager(QThread *thread, QObject *parent)
    : QObject(parent)
    , d_ptr(new GtUserManagerPrivate(this, thread))
{
}

GtUserManager::~GtUserManager()
{
}

void GtUserManager::connect(const QHostAddress &address, quint16 port)
{
}

void GtUserManager::disconnect()
{
}

GT_END_NAMESPACE
