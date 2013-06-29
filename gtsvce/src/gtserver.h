/*
 * Copyright (C) 2013 Tom Wong. All rights reserved.
 */
#ifndef __GT_SERVER_H__
#define __GT_SERVER_H__

#include "gtcommon.h"
#include <QtCore/QObject>
#include <QtNetwork/QHostAddress>

GT_BEGIN_NAMESPACE

class GtServerPrivate;

class GT_SVCE_EXPORT GtServer : public QObject
{
    Q_OBJECT

public:
    explicit GtServer(QObject *parent = 0);
    ~GtServer();

public:
    bool listen(quint16 port, const QHostAddress &address = QHostAddress::Any);

private Q_SLOTS:
    void handleNewConnection();

private:
    QScopedPointer<GtServerPrivate> d_ptr;

private:
    Q_DISABLE_COPY(GtServer)
    Q_DECLARE_PRIVATE(GtServer)
};

GT_END_NAMESPACE

#endif  /* __GT_SERVER_H__ */
