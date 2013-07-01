/*
 * Copyright (C) 2013 Tom Wong. All rights reserved.
 */
#ifndef __GT_SESSION_MANAGER_H__
#define __GT_SESSION_MANAGER_H__

#include "gtcommon.h"
#include <QtCore/QObject>

class QAbstractSocket;

GT_BEGIN_NAMESPACE

class GtSessionManagerPrivate;

class GT_SVCE_EXPORT GtSessionManager : public QObject
{
    Q_OBJECT

public:
    explicit GtSessionManager(QObject *parent = 0);
    ~GtSessionManager();

public:
    void setMaxThread(int count);
    void addSession(qintptr socketDescriptor);

private:
    QScopedPointer<GtSessionManagerPrivate> d_ptr;

private:
    Q_DISABLE_COPY(GtSessionManager)
    Q_DECLARE_PRIVATE(GtSessionManager)
};

GT_END_NAMESPACE

#endif  /* __GT_SESSION_MANAGER_H__ */
