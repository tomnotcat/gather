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
#include <QtCore/QUuid>

GT_BEGIN_NAMESPACE

class GtUserSessionPrivate
{
    Q_DECLARE_PUBLIC(GtUserSession)

public:
    GtUserSessionPrivate(GtUserSession *q);
    ~GtUserSessionPrivate();

public:
    void handleLogin(GtUserLoginRequest &msg);
    void handleLogout(GtUserLogoutRequest &msg);

protected:
    GtUserSession *q_ptr;
    QString m_sessionId;
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

    m_loggedIn = (GtUserClient::ErrorNone == error);
    if (m_loggedIn) {
        m_sessionId = QUuid::createUuid().toString();
        response.set_session_id(m_sessionId.toUtf8());
    }

    GtSvcUtil::sendMessage(q->socket(), GT_USER_LOGIN_RESPONSE, &response);
}

void GtUserSessionPrivate::handleLogout(GtUserLogoutRequest &msg)
{
    Q_UNUSED(msg);
    m_loggedIn = false;
    m_sessionId.clear();
}

GtUserSession::GtUserSession(QObject *parent)
    : GtSession(parent)
    , d_ptr(new GtUserSessionPrivate(this))
{
}

GtUserSession::~GtUserSession()
{
}

bool GtUserSession::isLoggedIn() const
{
    Q_D(const GtUserSession);
    return d->m_loggedIn;
}

QString GtUserSession::sessionId() const
{
    Q_D(const GtUserSession);
    return d->m_sessionId;
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

    case GT_USER_LOGOUT_REQUEST:
        {
            GtUserLogoutRequest msg;
            if (msg.ParseFromArray(data, size)) {
                d->handleLogout(msg);
            }
            else {
                qWarning() << "Invalid user logout request";
            }
        }
        break;

    default:
        qWarning() << "Unknown user message type:" << type;
        break;
    }
}

GT_END_NAMESPACE
