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
    class Account {
    public:
        explicit Account()
            : autoLogin(false)
        {
        }

        Account(const QString &u, const QString &p, bool r, bool a)
            : username(u)
            , password(p)
            , rememberMe(r)
            , autoLogin(a)
        {
        }

    public:
        QString username;
        QString password;
        bool rememberMe;
        bool autoLogin;
    };

public:
    explicit GtUserManager(const QString &regdb = QString(),
                           QThread *thread = 0,
                           QObject *parent = 0);
    ~GtUserManager();

public:
    QList<Account> accounts() const;

    void login(const Account &account);
    void logout();

    QString session() const;
    int state() const;

Q_SIGNALS:
    void loginError(int error);
    void stateChanged(int state, int error);

private Q_SLOTS:
    void loginSuccess(const QString &username,
                      const QString &password,
                      bool rememberMe,
                      bool autoLogin);

private:
    QScopedPointer<GtUserManagerPrivate> d_ptr;

private:
    Q_DISABLE_COPY(GtUserManager)
    Q_DECLARE_PRIVATE(GtUserManager)
};

GT_END_NAMESPACE

#endif  /* __GT_USER_MANAGER_H__ */
