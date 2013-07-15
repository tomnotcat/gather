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
    QString name;
    bool logined;
};

GtUserSessionPrivate::GtUserSessionPrivate(GtUserSession *q)
    : q_ptr(q)
    , logined(false)
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
    GtUserClient::LoginResult result;

    if (user.isEmpty()) {
        result = GtUserClient::InvalidUser;
    }
    else if (passwd.isEmpty()) {
        result = GtUserClient::InvalidPasswd;
    }
    else {
        result = GtUserClient::LoginSuccess;
    }

    GtUserLoginResponse response;
    response.set_result(result);
    GtSvcUtil::sendMessage(q->socket(), GT_USER_LOGIN_RESPONSE, &response);

    if (GtUserClient::LoginSuccess == result) {
        GtUserServer *server = qobject_cast<GtUserServer*>(q->server());
        this->name = user;
        server->addLogin(q);
    }

    logined = (GtUserClient::LoginSuccess == result);
}

GtUserSession::GtUserSession(QObject *parent)
    : GtSession(parent)
    , d_ptr(new GtUserSessionPrivate(this))
{
}

GtUserSession::~GtUserSession()
{
}

QString GtUserSession::name() const
{
    Q_D(const GtUserSession);
    return d->name;
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
