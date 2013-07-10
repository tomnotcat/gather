/*
 * Copyright (C) 2013 Tom Wong. All rights reserved.
 */
#ifndef __GT_SERVER_P_H__
#define __GT_SERVER_P_H__

#include "gtserver.h"
#include <QtCore/QMutex>
#include <QtCore/QThread>

GT_BEGIN_NAMESPACE

class GtServerThread : public QThread, public GtObject
{
private:
    friend class GtServerPrivate;
    QList<GtSession*> sessions;
};

class GtServerPrivate
{
    Q_DECLARE_PUBLIC(GtServer)

public:
    explicit GtServerPrivate(GtServer *q);
    ~GtServerPrivate();

public:
    GtServerThread* fetchThread();
    void addSession(GtSession *session, qintptr socketDescriptor);
    void removeSession(GtSession *session);
    void close();

protected:
    GtServer *q_ptr;
    QList<GtServerThread*> threads;
    QMutex mutex;
    int maxThread;
    bool closing;
};

GT_END_NAMESPACE

#endif  /* __GT_SERVER_P_H__ */
