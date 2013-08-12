/*
 * Copyright (C) 2013 Tom Wong. All rights reserved.
 */
#ifndef __GT_SESSION_H__
#define __GT_SESSION_H__

#include "gtobject.h"
#include <QtCore/QObject>
#include <QtNetwork/QAbstractSocket>

class QAbstractSocket;

GT_BEGIN_NAMESPACE

class GtServer;
class GtSessionPrivate;

class GT_SVCE_EXPORT GtSession : public QObject, public GtObject
{
    Q_OBJECT

public:
    explicit GtSession(QObject *parent = 0);
    ~GtSession();

public:
    virtual void message(const char *data, int size);

public:
    GtServer* server() const;
    QAbstractSocket* socket() const;
    void close();

    QHostAddress peerAddress() const;
    quint16 peerPort() const;

private Q_SLOTS:
    void handleRead();
    void handleError(QAbstractSocket::SocketError error);

private:
    friend class GtServerPrivate;
    QScopedPointer<GtSessionPrivate> d_ptr;

private:
    Q_DISABLE_COPY(GtSession)
    Q_DECLARE_PRIVATE(GtSession)
};

GT_END_NAMESPACE

#endif  /* __GT_SESSION_H__ */
