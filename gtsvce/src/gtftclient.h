/*
 * Copyright (C) 2013 Tom Wong. All rights reserved.
 */
#ifndef __GT_FT_CLIENT_H__
#define __GT_FT_CLIENT_H__

#include "gtobject.h"
#include <QtCore/QObject>
#include <QtNetwork/QAbstractSocket>

GT_BEGIN_NAMESPACE

class GtFTFile;
class GtFTClientPrivate;

class GT_SVCE_EXPORT GtFTClient : public QObject, public GtObject
{
    Q_OBJECT

public:
    enum LoginResult {
        LoginSuccess,
        InvalidSession,
        LoginUnknown = -1
    };

    enum LogoutReason {
        LogoutNormal,
        LogoutUnknown = -1
    };

public:
    explicit GtFTClient(QObject *parent = 0);
    ~GtFTClient();

public:
    void login(const QHostAddress &address, quint16 port,
               const QString &session, const QString &secret);
    void logout();
    GtFTFile* openFile(const QString &fileId);

Q_SIGNALS:
    void login(int result);
    void logout(int reason);

private Q_SLOTS:
    void realLogin();
    void realLogout();
    void handleRead();
    void handleConnected();
    void handleDisconnected();
    void handleError(QAbstractSocket::SocketError error);

private:
    QScopedPointer<GtFTClientPrivate> d_ptr;

private:
    Q_DISABLE_COPY(GtFTClient)
    Q_DECLARE_PRIVATE(GtFTClient)
};

GT_END_NAMESPACE

#endif  /* __GT_FT_CLIENT_H__ */
