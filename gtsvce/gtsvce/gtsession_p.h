/*
 * Copyright (C) 2013 Tom Wong. All rights reserved.
 */
#ifndef __GT_SESSION_P_H__
#define __GT_SESSION_P_H__

#include "gtsession.h"
#include "gtrecvbuffer.h"

GT_BEGIN_NAMESPACE

class GtServer;
class GtServerThread;

class GtSessionPrivate
{
    Q_DECLARE_PUBLIC(GtSession)

public:
    explicit GtSessionPrivate(GtSession *q);
    ~GtSessionPrivate();

public:
    void init(QAbstractSocket *s, GtServer *m, GtServerThread *t);

protected:
    friend class GtServerPrivate;

protected:
    GtSession *q_ptr;
    QAbstractSocket *socket;
    GtServer *server;
    GtServerThread *thread;
    GtRecvBuffer buffer;
};

GT_END_NAMESPACE

#endif  /* __GT_SESSION_P_H__ */
