/*
 * Copyright (C) 2013 Tom Wong. All rights reserved.
 */
#include "gtftsession.h"
#include "gtftclient.h"
#include "gtftmessage.pb.h"
#include "gtsvcutil.h"
#include <QtCore/QDebug>
#include <QtCore/qendian.h>

GT_BEGIN_NAMESPACE

class GtFTSessionPrivate
{
    Q_DECLARE_PUBLIC(GtFTSession)

public:
    GtFTSessionPrivate(GtFTSession *q);
    ~GtFTSessionPrivate();

public:
    void handleLoginRequest(GtFTLoginRequest &msg);

protected:
    GtFTSession *q_ptr;
    bool logined;
};

GtFTSessionPrivate::GtFTSessionPrivate(GtFTSession *q)
    : q_ptr(q)
    , logined(false)
{
}

GtFTSessionPrivate::~GtFTSessionPrivate()
{
}

void GtFTSessionPrivate::handleLoginRequest(GtFTLoginRequest &msg)
{
    Q_Q(GtFTSession);

    QString session = QString::fromUtf8(msg.session().c_str());
    GtFTClient::LoginResult result;

    if (session.isEmpty()) {
        result = GtFTClient::InvalidSession;
    }
    else {
        result = GtFTClient::LoginSuccess;
    }

    GtFTLoginResponse response;
    response.set_result(result);
    GtSvcUtil::sendMessage(q->socket(), GT_FT_LOGIN_RESPONSE, &response);

    logined = (GtFTClient::LoginSuccess == result);
}

GtFTSession::GtFTSession(QObject *parent)
    : GtSession(parent)
    , d_ptr(new GtFTSessionPrivate(this))
{
}

GtFTSession::~GtFTSession()
{
}

void GtFTSession::message(const char *data, int size)
{
    Q_D(GtFTSession);

    if (size < (int)sizeof(quint16)) {
        qWarning() << "Invalid message size:" << size;
        return;
    }

    quint16 type = qFromBigEndian<quint16>(*(quint16*)data);
    data += sizeof(quint16);
    size -= sizeof(quint16);

    switch (type) {
    case GT_FT_LOGIN_REQUEST:
        {
            GtFTLoginRequest msg;
            if (msg.ParseFromArray(data, size)) {
                d->handleLoginRequest(msg);
            }
            else {
                qWarning() << "Invalid FT login request";
            }
        }
        break;

    default:
        qWarning() << "Invalid FT message:" << type;
        break;
    }
}

GT_END_NAMESPACE
