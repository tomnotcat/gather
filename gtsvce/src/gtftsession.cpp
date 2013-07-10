/*
 * Copyright (C) 2013 Tom Wong. All rights reserved.
 */
#include "gtftsession.h"
#include "gtdocument.h"
#include "gtftclient.h"
#include "gtftmessage.pb.h"
#include "gtftserver.h"
#include "gtfttemp.h"
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
    void close();

public:
    void handleOpenRequest(GtFTOpenRequest &msg);
    void handleWriteRequest(const char *data, int size);

protected:
    GtFTSession *q_ptr;
    GtFTTemp temp;
    bool opened;
};

GtFTSessionPrivate::GtFTSessionPrivate(GtFTSession *q)
    : q_ptr(q)
    , opened(false)
{
}

GtFTSessionPrivate::~GtFTSessionPrivate()
{
    close();
}

void GtFTSessionPrivate::close()
{
    if (!opened)
        return;

    temp.seek(0);

    QString fileId(GtDocument::makeFileId(&temp));
    bool finished = (temp.fileId() == fileId);

    temp.close();
    opened = false;

    if (finished) {
        Q_Q(GtFTSession);

        GtFTServer *server = qobject_cast<GtFTServer*>(q->server());
        emit server->uploaded(fileId);
    }
}

void GtFTSessionPrivate::handleOpenRequest(GtFTOpenRequest &msg)
{
    Q_Q(GtFTSession);

    QString session = QString::fromUtf8(msg.session().c_str());
    QString fileId = QString::fromUtf8(msg.fileid().c_str());
    GtFTClient::ErrorCode result;

    if (opened) {
        result = GtFTClient::InvalidState;
    }
    else if (session.isEmpty()) {
        result = GtFTClient::InvalidSession;
    }
    else {
        GtFTServer *server = qobject_cast<GtFTServer*>(q->server());
        temp.setPath(server->tempPath(), fileId);
        opened = temp.open(QIODevice::ReadWrite);

        if (opened) {
            result = GtFTClient::NoError;
        }
        else {
            qWarning() << "open temp file failed:" << fileId;
            result = GtFTClient::OpenFailed;
        }
    }

    GtFTOpenResponse response;
    response.set_error(result);
    GtSvcUtil::sendMessage(q->socket(), GT_FT_OPEN_RESPONSE, &response);
}

void GtFTSessionPrivate::handleWriteRequest(const char *data, int size)
{
    Q_Q(GtFTSession);

    GtFTWriteResponse response;

    if (opened) {
        qint64 len = temp.write(data, size);
        response.set_size(len);
    }
    else {
        response.set_size(-1);
    }

    GtSvcUtil::sendMessage(q->socket(), GT_FT_WRITE_RESPONSE, &response);
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

    case GT_FT_WRITE_REQUEST:
        d->handleWriteRequest(data, size);
        break;

    default:
        qWarning() << "Invalid FT message:" << type;
        break;
    }
}

GT_END_NAMESPACE
