/*
 * Copyright (C) 2013 Tom Wong. All rights reserved.
 */
#ifndef __GT_SERVER_H__
#define __GT_SERVER_H__

#include "gtobject.h"
#include <QtCore/QObject>
#include <QtNetwork/QTcpServer>

GT_BEGIN_NAMESPACE

class GtSessionManager;

class GT_SVCE_EXPORT GtServer : public QTcpServer, public GtObject
{
    Q_OBJECT

public:
    explicit GtServer(QObject *parent = 0);
    ~GtServer();

protected:
    void incomingConnection(qintptr socketDescriptor);

private:
    GtSessionManager *sessionManager;

private:
    Q_DISABLE_COPY(GtServer)
};

GT_END_NAMESPACE

#endif  /* __GT_SERVER_H__ */
