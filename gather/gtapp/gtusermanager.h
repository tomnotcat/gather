/*
 * Copyright (C) 2013 Tom Wong. All rights reserved.
 */
#ifndef __GT_USER_MANAGER_H__
#define __GT_USER_MANAGER_H__

#include "gtobject.h"
#include <QtCore/QObject>

GT_BEGIN_NAMESPACE

class GtUserManagerPrivate;

class GT_APP_EXPORT GtUserManager : public QObject, public GtObject
{
    Q_OBJECT;

public:
    explicit GtUserManager(QThread *thread = 0, QObject *parent = 0);
    ~GtUserManager();

public:
    void connect(const QString &host, quint16 port);
    void disconnect();

    void login(const QString &user, const QString &passwd);
    void logout();

    QString session() const;
    int state() const;

Q_SIGNALS:
    void connectError(int error);
    void loginError(int error);
    void stateChanged(int state, int error);

private:
    QScopedPointer<GtUserManagerPrivate> d_ptr;

private:
    Q_DISABLE_COPY(GtUserManager)
    Q_DECLARE_PRIVATE(GtUserManager)
};

GT_END_NAMESPACE

#endif  /* __GT_USER_MANAGER_H__ */
