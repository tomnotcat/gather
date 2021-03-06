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
    enum ErrorCode {
        ErrorNone,
        ErrorHostNotFound,
        ErrorDisconnected,
        ErrorSendFail,
        ErrorReceiveFail,
        ErrorInvalidMessage,
        ErrorInvalidState,
        ErrorInvalidUser,
        ErrorInvalidPasswd
    };

    enum StateCode {
        UnconnectedState,
        ConnectingState,
        ConnectedState,
        LoggingInState,
        LoggedInState
    };

public:
    explicit GtUserClient(QObject *parent = 0);
    ~GtUserClient();

public:
    bool connect(const QHostAddress &address, quint16 port);
    void disconnect();

    bool login(const QString &user, const QString &passwd);
    void logout();

    QString session() const;

    ErrorCode error() const;
    StateCode state() const;

    void clearError();

Q_SIGNALS:
    void stateChanged(int state);

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
