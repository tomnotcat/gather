/*
 * Copyright (C) 2013 Tom Wong. All rights reserved.
 */
#include "gtusersession.h"
#include "gtuserclient.h"
#include "gtusermessage.pb.h"
#include "gtuserserver.h"
#include "gtsvcutil.h"
#include <QtCore/QDebug>
#include <QtCore/qendian.h>

GT_BEGIN_NAMESPACE

class GtUserSessionPrivate
{
    Q_DECLARE_PUBLIC(GtUserSession)

public:
    GtUserSessionPrivate(GtUserSession *q);
    ~GtUserSessionPrivate();

public:
    void handleLogin(GtUserLoginRequest &msg);

protected:
    GtUserSession *q_ptr;
    bool m_loggedIn;
};

GtUserSessionPrivate::GtUserSessionPrivate(GtUserSession *q)
    : q_ptr(q)
    , m_loggedIn(false)
{
}

GtUserSessionPrivate::~GtUserSessionPrivate()
{
}

void GtUserSessionPrivate::handleLogin(GtUserLoginRequest &msg)
{
    Q_Q(GtUserSession);

    QString user = QString::fromUtf8(msg.user().c_str());
    QString passwd = QString::fromUtf8(msg.passwd().c_str());
    GtUserClient::ErrorCode error = GtUserClient::ErrorNone;

    if (user.isEmpty()) {
        error = GtUserClient::ErrorInvalidUser;
    }
    else if (passwd.isEmpty()) {
        error = GtUserClient::ErrorInvalidPasswd;
    }

    GtUserLoginResponse response;
    response.set_error(error);
    GtSvcUtil::sendMessage(q->socket(), GT_USER_LOGIN_RESPONSE, &response);

    m_loggedIn = (GtUserClient::ErrorNone == error);
    if (m_loggedIn) {
        GtUserServer *server = qobject_cast<GtUserServer*>(q->server());
        m_user = user;
        m_passwd = passwd;
        server->addLogin(q);
    }
}

GtUserSession::GtUserSession(QObject *parent)
    : GtSession(parent)
    , d_ptr(new GtUserSessionPrivate(this))
{
}

GtUserSession::~GtUserSession()
{
}

void GtUserSession::message(const char *data, int size)
{
    Q_D(GtUserSession);

    if (size < (int)sizeof(quint16)) {
        qWarning() << "Invalid message size:" << size;
        return;
    }

    quint16 type = qFromBigEndian<quint16>(*(quint16*)data);
    data += sizeof(quint16);
    size -= sizeof(quint16);

    switch (type) {
    case GT_USER_LOGIN_REQUEST:
        {
            GtUserLoginRequest msg;
            if (msg.ParseFromArray(data, size)) {
                d->handleLogin(msg);
            }
            else {
                qWarning() << "Invalid user login request";
            }
        }
        break;

    default:
        qWarning() << "Invalid user message type:" << type;
        break;
    }
}

void GtUserSession::reloginLogout()
{
    GtUserLogoutResponse msg;
    msg.set_reason(GtUserClient::LogoutRelogin);
    GtSvcUtil::sendMessage(socket(), GT_USER_LOGOUT_RESPONSE, &msg);
}

GT_END_NAMESPACE
