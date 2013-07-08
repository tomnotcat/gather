/*
 * Copyright (C) 2013 Tom Wong. All rights reserved.
 */
#ifndef __GT_USER_CLIENT_H__
#define __GT_USER_CLIENT_H__

#include "gtobject.h"
#include <QtCore/QObject>
#include <QtNetwork/QAbstractSocket>

GT_BEGIN_NAMESPACE

class GtUserClientPrivate;

class GT_SVCE_EXPORT GtUserClient : public QObject, public GtObject
{
    Q_OBJECT

public:
    explicit GtUserClient(QObject *parent = 0);
    ~GtUserClient();

public:
    void login(const QHostAddress &address, quint16 port,
               const QString &user, const QString &passwd);
    void logout();

public:
    enum LoginResult {
        LoginSuccess,
        InvalidUser,
        InvalidPasswd,
        LoginUnknown = -1
    };

    enum LogoutReason {
        LogoutNormal,
        LogoutRelogin,
        LogoutUnknown = -1
    };

Q_SIGNALS:
    void login(int result);
    void logout(int reason);

private Q_SLOTS:
    void handleRead();
    void handleConnected();
    void handleDisconnected();
    void handleError(QAbstractSocket::SocketError error);

private:
    QScopedPointer<GtUserClientPrivate> d_ptr;

private:
    Q_DISABLE_COPY(GtUserClient)
    Q_DECLARE_PRIVATE(GtUserClient)
};

GT_END_NAMESPACE

#endif  /* __GT_USER_CLIENT_H__ */
