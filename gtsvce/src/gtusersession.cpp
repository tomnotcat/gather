/*
 * Copyright (C) 2013 Tom Wong. All rights reserved.
 */
#include "gtusersession.h"
#include "gtclient.h"
#include "gtmessage.pb.h"
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
    GtSimpleMessage response;

    if (user.isEmpty()) {
        response.set_data1(GtClient::InvalidUser);
    }
    else if (passwd.isEmpty()) {
        response.set_data1(GtClient::InvalidPasswd);
    }
    else {
        response.set_data1(GtClient::LoginSuccess);
    }

    GtSvcUtil::sendMessage(q->socket(), GT_LOGIN_RESPONSE, response);
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

GT_END_NAMESPACE
