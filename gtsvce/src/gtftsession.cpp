/*
 * Copyright (C) 2013 Tom Wong. All rights reserved.
 */
#include "gtftsession.h"
#include "gtftmessage.pb.h"
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
    void handleUploadRequest(GtFTUploadRequest &msg);

protected:
    GtFTSession *q_ptr;
};

GtFTSessionPrivate::GtFTSessionPrivate(GtFTSession *q)
    : q_ptr(q)
{
}

GtFTSessionPrivate::~GtFTSessionPrivate()
{
}

void GtFTSessionPrivate::handleUploadRequest(GtFTUploadRequest &msg)
{
    qDebug() << "++++++++++++++++++++++++++++++++++";
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
    case GT_UPLOAD_REQUEST:
        {
            GtFTUploadRequest msg;
            if (msg.ParseFromArray(data, size)) {
                d->handleUploadRequest(msg);
            }
            else {
                qWarning() << "Invalid upload request";
            }
        }
        break;

    default:
        qWarning() << "Invalid FT message:" << type;
        break;
    }
}

GT_END_NAMESPACE
