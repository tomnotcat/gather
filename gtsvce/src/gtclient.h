/*
 * Copyright (C) 2013 Tom Wong. All rights reserved.
 */
#ifndef __GT_CLIENT_H__
#define __GT_CLIENT_H__

#include "gtcommon.h"
#include <QtCore/QObject>
#include <QtNetwork/QAbstractSocket>

GT_BEGIN_NAMESPACE

class GtClientPrivate;

class GT_SVCE_EXPORT GtClient : public QObject
{
    Q_OBJECT

public:
    explicit GtClient(QObject *parent = 0);
    ~GtClient();

public:
    void login(const QHostAddress &address, quint16 port,
               const QString &user, const QString &passwd);

public:
    enum LoginResult {
        LoginSuccess,
        InvalidUser,
        InvalidPasswd
    };

Q_SIGNALS:
    void onLogin(LoginResult result);

private Q_SLOTS:
    void handleRead();
    void handleConnected();
    void handleDisconnected();
    void handleError(QAbstractSocket::SocketError error);

private:
    QScopedPointer<GtClientPrivate> d_ptr;

private:
    Q_DISABLE_COPY(GtClient)
    Q_DECLARE_PRIVATE(GtClient)
};

GT_END_NAMESPACE

#endif  /* __GT_CLIENT_H__ */
