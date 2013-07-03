/*
 * Copyright (C) 2013 Tom Wong. All rights reserved.
 */
#include "gtusersession.h"
#include "gtclient.h"
#include "gtmessage.pb.h"
#include "gtuserserver.h"
#include "gtsvcutil.h"
#include <QtCore/QDebug>

GT_BEGIN_NAMESPACE

class GtUserSessionPrivate
{
    Q_DECLARE_PUBLIC(GtUserSession)

public:
    GtUserSessionPrivate(GtUserSession *q);
    ~GtUserSessionPrivate();

public:
    void handleLogin(GtLoginRequest &msg);

protected:
    GtUserSession *q_ptr;
    QString name;
};

GtUserSessionPrivate::GtUserSessionPrivate(GtUserSession *q)
    : q_ptr(q)
{
}

GtUserSessionPrivate::~GtUserSessionPrivate()
{
}

void GtUserSessionPrivate::handleLogin(GtLoginRequest &msg)
{
    Q_Q(GtUserSession);

    QString user = QString::fromUtf8(msg.user().c_str());
    QString passwd = QString::fromUtf8(msg.passwd().c_str());
    GtClient::LoginResult result = GtClient::LoginUnknown;

    if (user.isEmpty()) {
        result = GtClient::InvalidUser;
    }
    else if (passwd.isEmpty()) {
        result = GtClient::InvalidPasswd;
    }
    else {
        result = GtClient::LoginSuccess;
    }

    GtSimpleMessage response;
    response.set_data1(result);
    GtSvcUtil::sendMessage(q->socket(), GT_LOGIN_RESPONSE, response);

    if (GtClient::LoginSuccess == result) {
        GtUserServer *server = qobject_cast<GtUserServer*>(q->server());
        this->name = user;
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

    quint16 type = ntohs(*(quint16*)data);
    data += sizeof(quint16);
    size -= sizeof(quint16);

    switch (type) {
    case GT_LOGIN_REQUEST:
        {
            GtLoginRequest msg;
            if (msg.ParseFromArray(data, size)) {
                d->handleLogin(msg);
            }
            else {
                qWarning() << "Invalid login request";
            }
        }
        break;

    default:
        qWarning() << "Invalid message type:" << type;
        break;
    }
}

void GtUserSession::reloginLogout()
{
    GtSimpleMessage msg;
    msg.set_data1(GtClient::LogoutRelogin);
    GtSvcUtil::sendMessage(socket(), GT_LOGOUT_MESSAGE, msg);
}

GT_END_NAMESPACE
