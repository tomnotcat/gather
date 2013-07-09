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
    void handleOpenRequest(GtFTOpenRequest &msg);

protected:
    GtFTSession *q_ptr;
    bool opened;
};

GtFTSessionPrivate::GtFTSessionPrivate(GtFTSession *q)
    : q_ptr(q)
    , opened(false)
{
}

GtFTSessionPrivate::~GtFTSessionPrivate()
{
}

void GtFTSessionPrivate::handleOpenRequest(GtFTOpenRequest &msg)
{
    Q_Q(GtFTSession);

    QString session = QString::fromUtf8(msg.session().c_str());
    GtFTClient::ErrorCode result;

    if (session.isEmpty()) {
        result = GtFTClient::InvalidSession;
    }
    else {
        result = GtFTClient::NoError;
    }

    GtFTOpenResponse response;
    response.set_result(result);
    response.set_size(0);
    GtSvcUtil::sendMessage(q->socket(), GT_FT_OPEN_RESPONSE, &response);

    opened = (GtFTClient::NoError == result);
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
    case GT_FT_OPEN_REQUEST:
        {
            GtFTOpenRequest msg;
            if (msg.ParseFromArray(data, size)) {
                d->handleOpenRequest(msg);
            }
            else {
                qWarning() << "Invalid FT open request";
            }
        }
        break;

    default:
        qWarning() << "Invalid FT message:" << type;
        break;
    }
}

GT_END_NAMESPACE
