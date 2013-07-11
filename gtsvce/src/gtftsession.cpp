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
    bool complete();
    void close();

public:
    void handleOpenRequest(GtFTOpenRequest &msg);
    void handleSeekRequest(GtFTSeekRequest &msg);
    void handleSizeRequest();
    void handleWriteRequest(const char *data, int size);
    void handleCompleteRequest();

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

bool GtFTSessionPrivate::complete()
{
    qint64 pos = temp.pos();
    temp.seek(0);

    QString fileId(GtDocument::makeFileId(&temp));

    temp.seek(pos);
    return (temp.fileId() == fileId);
}

void GtFTSessionPrivate::close()
{
    if (!opened)
        return;

    bool complete = this->complete();

    temp.close();
    opened = false;

    if (complete) {
        Q_Q(GtFTSession);

        GtFTServer *server = qobject_cast<GtFTServer*>(q->server());
        emit server->uploaded(temp.fileId());
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

    if (GtFTClient::NoError == result) {
        for (int i = 0; i < temp.temps_size(); ++i) {
            *response.add_temps() = temp.temps(i);
        }
    }

    GtSvcUtil::sendMessage(q->socket(), GT_FT_OPEN_RESPONSE, &response);
}

void GtFTSessionPrivate::handleSeekRequest(GtFTSeekRequest &msg)
{
    Q_Q(GtFTSession);

    GtFTSeekResponse response;

    if (opened) {
        if (temp.seek(msg.pos()))
            response.set_error(GtFTClient::NoError);
        else
            response.set_error(GtFTClient::SeekFailed);
    }
    else {
        response.set_error(GtFTClient::InvalidState);
    }

    GtSvcUtil::sendMessage(q->socket(), GT_FT_SEEK_RESPONSE, &response);
}

void GtFTSessionPrivate::handleSizeRequest()
{
    Q_Q(GtFTSession);

    GtFTSizeResponse response;

    if (opened) {
        response.set_size(temp.size());
    }
    else {
        response.set_size(-1);
    }

    GtSvcUtil::sendMessage(q->socket(), GT_FT_SIZE_RESPONSE, &response);
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

void GtFTSessionPrivate::handleCompleteRequest()
{
    Q_Q(GtFTSession);

    GtFTCompleteResponse response;

    if (opened) {
        if (complete())
            response.set_error(GtFTClient::NoError);
        else
            response.set_error(GtFTClient::InvalidState);
    }
    else {
        response.set_error(GtFTClient::InvalidState);
    }

    GtSvcUtil::sendMessage(q->socket(), GT_FT_COMPLETE_RESPONSE, &response);
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

    case GT_FT_SEEK_REQUEST:
        {
            GtFTSeekRequest msg;
            if (msg.ParseFromArray(data, size)) {
                d->handleSeekRequest(msg);
            }
            else {
                qWarning() << "Invalid FT seek request";
            }
        }
        break;

    case GT_FT_SIZE_REQUEST:
        if (0 == size) {
            d->handleSizeRequest();
        }
        else {
            qWarning() << "Invalid FT size request";
        }
        break;

    case GT_FT_WRITE_REQUEST:
        d->handleWriteRequest(data, size);
        break;

    case GT_FT_COMPLETE_REQUEST:
        if (0 == size) {
            d->handleCompleteRequest();
        }
        else {
            qWarning() << "Invalid FT complete request";
        }
        break;

    default:
        qWarning() << "Invalid FT message:" << type;
        break;
    }
}

GT_END_NAMESPACE
