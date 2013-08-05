/*
 * Copyright (C) 2013 Tom Wong. All rights reserved.
 */
#ifndef __GT_USER_CLIENT_H__
#define __GT_USER_CLIENT_H__

#include "gtobject.h"
#include <QtCore/QObject>
#include <QtNetwork/QAbstractSocket>

GT_BEGIN_NAMESPACE

class GtBookmarks;
class GtDocMeta;
class GtDocNotes;
class GtUserBookmarks;
class GtUserDocMeta;
class GtUserDocNotes;
class GtUserClientPrivate;

class GT_SVCE_EXPORT GtUserClient : public QObject, public GtObject
{
    Q_OBJECT

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

public:
    explicit GtUserClient(QObject *parent = 0);
    ~GtUserClient();

public:
    void login(const QHostAddress &address, quint16 port,
               const QString &user, const QString &passwd);
    void logout();

public:
    static bool convert(const GtDocMeta &src, GtUserDocMeta &dest);
    static bool convert(const GtUserDocMeta &src, GtDocMeta &dest);
    static bool convert(const GtBookmarks &src, GtUserBookmarks &dest);
    static bool convert(const GtUserBookmarks &src, GtBookmarks &dest);
    static bool convert(const GtDocNotes &src, GtUserDocNotes &dest);
    static bool convert(const GtUserDocNotes &src, GtDocNotes &dest);

    template<typename TS, typename TD>
    static bool serialize(const TS &src, QByteArray &data)
    {
        TD dest;
        if (!GtUserClient::convert(src, dest))
            return false;

        data.resize(dest.ByteSize());
        return dest.SerializeToArray(data.data(), data.size());
    }

    template<typename TS, typename TD>
    static bool deserialize(const QByteArray &data, TD &dest)
    {
        TS src;
        if (!src.ParseFromArray(data.data(), data.size()))
            return false;

        return GtUserClient::convert(src, dest);
    }

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
