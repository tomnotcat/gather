/*
 * Copyright (C) 2013 Tom Wong. All rights reserved.
 */
#ifndef __GT_SERVER_H__
#define __GT_SERVER_H__

#include "gtobject.h"
#include <QtCore/QObject>
#include <QtNetwork/QTcpServer>

GT_BEGIN_NAMESPACE

class GtSession;
class GtServerPrivate;

class GT_SVCE_EXPORT GtServer : public QTcpServer, public GtObject
{
    Q_OBJECT

public:
    explicit GtServer(QObject *parent = 0);
    ~GtServer();

public:
    void setMaxThread(int count);
    int maxThread() const;
    void close();

protected:
    virtual GtSession* createSession() = 0;
    virtual void addSession(GtSession *session);
    virtual void removeSession(GtSession *session);

protected:
    void incomingConnection(qintptr socketDescriptor);

private:
    friend class GtSession;

private:
    QScopedPointer<GtServerPrivate> d_ptr;

private:
    Q_DISABLE_COPY(GtServer)
    Q_DECLARE_PRIVATE(GtServer)
};

GT_END_NAMESPACE

#endif  /* __GT_SERVER_H__ */
