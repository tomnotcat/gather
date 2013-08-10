/*
 * Copyright (C) 2013 Tom Wong. All rights reserved.
 */
#ifndef __GT_USER_MANAGER_H__
#define __GT_USER_MANAGER_H__

#include "gtobject.h"
#include <QtCore/QObject>

class QHostAddress;

GT_BEGIN_NAMESPACE

class GtUserManagerPrivate;

class GT_APP_EXPORT GtUserManager : public QObject, public GtObject
{
    Q_OBJECT;

public:
    explicit GtUserManager(QThread *thread = 0, QObject *parent = 0);
    ~GtUserManager();

public:
    void connect(const QHostAddress &address, quint16 port);
    void disconnect();

Q_SIGNALS:
    void connected();
    void disconnected();

private:
    QScopedPointer<GtUserManagerPrivate> d_ptr;

private:
    Q_DISABLE_COPY(GtUserManager)
    Q_DECLARE_PRIVATE(GtUserManager)
};

GT_END_NAMESPACE

#endif  /* __GT_USER_MANAGER_H__ */
