/*
 * Copyright (C) 2013 Tom Wong. All rights reserved.
 */
#ifndef __GT_USER_MANAGER_P_H__
#define __GT_USER_MANAGER_P_H__

#include "gtusermanager.h"

GT_BEGIN_NAMESPACE

class GtUserManagerProxy : public QObject, public GtObject
{
    Q_OBJECT

public:
    GtUserManagerProxy(GtUserManagerPrivate *p);
    ~GtUserManagerProxy();

public:
    Q_INVOKABLE void connect(const QString &host, quint16 port);
    Q_INVOKABLE void disconnect();

    Q_INVOKABLE void login(const QString &user, const QString &passwd);
    Q_INVOKABLE void logout();

private Q_SLOTS:
    void stateChanged(int state);

private:
    GtUserManagerPrivate *m_priv;
};

GT_END_NAMESPACE

#endif  /* __GT_USER_MANAGER_P_H__ */
