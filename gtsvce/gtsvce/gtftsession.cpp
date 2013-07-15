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
#include <limits>

GT_BEGIN_NAMESPACE

class GtFTSessionPrivate
{
    Q_DECLARE_PUBLIC(GtFTSession)

public:
    GtFTSessionPrivate(GtFTSession *q);
    ~GtFTSessionPrivate();

public:
    bool finish();
    void close();

public:
    void handleOpenRequest(GtFTOpenRequest &msg);
    void handleSeekRequest(GtFTSeekRequest &msg);
    void handleSizeRequest();
    void handleReadRequest(GtFTReadRequest &msg);
    void handleWriteRequest(const char *data, int size);
    void handleFinishRequest();

protected:
    GtFTSession *q_ptr;
    GtFTTemp temp;
    QIODevice *device;
    bool opened;
};

GtFTSessionPrivate::GtFTSessionPrivate(GtFTSession *q)
    : q_ptr(q)
    , device(0)
    , opened(false)
{
}

GtFTSessionPrivate::~GtFTSessionPrivate()
{
    close();
}

bool GtFTSessionPrivate::finish()
{
    if (device != &temp)
        return true;

    qint64 pos = temp.pos();
    temp.seek(0);

    QString fileId(GtDocument::makeFileId(&temp));

    temp.seek(pos);
    return (temp.fileId() == fileId);
}

void GtFTSessionPrivate::close()
{
    Q_Q(GtFTSession);

    if (opened) {
        if (&temp == device && finish()) {
            GtFTServer *server = qobject_cast<GtFTServer*>(q->server());
            server->upload(temp.fileId(), device);
        }

        device->close();
        opened = false;
    }

    if (device) {
        if (device != &temp)
            delete device;

        device = 0;
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
        QIODevice::OpenMode mode;

        mode = static_cast<QIODevice::OpenMode>(msg.mode());
        mode |= QIODevice::ReadOnly;

        if (QIODevice::ReadOnly == mode) {
            // download
            device = server->download(fileId);
        }
        else {
            // upload
            temp.setPath(server->tempPath(), fileId);
            device = &temp;
        }

        if (device) {
            opened = device->open(mode);

            if (opened) {
                result = GtFTClient::NoError;
            }
            else {
                qWarning() << "open temp file failed:" << fileId;
                result = GtFTClient::OpenFailed;
            }
        }
        else {
            result = GtFTClient::FileNotExists;
        }
    }

    GtFTOpenResponse response;
    response.set_error(result);

    if (GtFTClient::NoError == result) {
        if (&temp == device) {
            for (int i = 0; i < temp.temps_size(); ++i) {
                *response.add_temps() = temp.temps(i);
            }
        }
        else {
            GtFTTempData *data = response.add_temps();
            data->set_offset(0);
            data->set_size(device->size());
        }
    }

    GtSvcUtil::sendMessage(q->socket(), GT_FT_OPEN_RESPONSE, &response);
}

void GtFTSessionPrivate::handleSeekRequest(GtFTSeekRequest &msg)
{
    Q_Q(GtFTSession);

    GtFTSeekResponse response;

    if (opened) {
        if (device->seek(msg.pos()))
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
        response.set_size(device->size());
    }
    else {
        response.set_size(-1);
    }

    GtSvcUtil::sendMessage(q->socket(), GT_FT_SIZE_RESPONSE, &response);
}

void GtFTSessionPrivate::handleReadRequest(GtFTReadRequest &msg)
{
    Q_Q(GtFTSession);

    int size = msg.size();
    if (size + sizeof(quint16) > std::numeric_limits<quint16>::max()) {
        qWarning() << "Invalid FT read size:" << size;
        size = 0;
    }

    QByteArray bytes(sizeof(quint32) + size, -1);
    char *buffer = bytes.data();

    if (size > 0)
        size = device->read(buffer + sizeof(quint32), size);

    *(quint16*)buffer = qToBigEndian<quint16>(static_cast<quint16>(size + sizeof(quint16)));
    *(quint16*)(buffer + sizeof(quint16)) = qToBigEndian<quint16>(GT_FT_READ_RESPONSE);

    GtSvcUtil::syncWrite(q->socket(), buffer, sizeof(quint32) + size);
}

void GtFTSessionPrivate::handleWriteRequest(const char *data, int size)
{
    Q_Q(GtFTSession);

    GtFTWriteResponse response;

    if (opened) {
        qint64 len = device->write(data, size);
        response.set_size(len);
    }
    else {
        response.set_size(-1);
    }

    GtSvcUtil::sendMessage(q->socket(), GT_FT_WRITE_RESPONSE, &response);
}

void GtFTSessionPrivate::handleFinishRequest()
{
    Q_Q(GtFTSession);

    GtFTFinishResponse response;

    if (opened) {
        if (finish())
            response.set_error(GtFTClient::NoError);
        else
            response.set_error(GtFTClient::InvalidState);
    }
    else {
        response.set_error(GtFTClient::InvalidState);
    }

    GtSvcUtil::sendMessage(q->socket(), GT_FT_FINISH_RESPONSE, &response);
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

    case GT_FT_READ_REQUEST:
        {
            GtFTReadRequest msg;
            if (msg.ParseFromArray(data, size)) {
                d->handleReadRequest(msg);
            }
            else {
                qWarning() << "Invalid FT read request";
            }
        }
        break;

    case GT_FT_WRITE_REQUEST:
        d->handleWriteRequest(data, size);
        break;

    case GT_FT_FINISH_REQUEST:
        if (0 == size) {
            d->handleFinishRequest();
        }
        else {
            qWarning() << "Invalid FT finish request";
        }
        break;

    default:
        qWarning() << "Invalid FT message:" << type;
        break;
    }
}

GT_END_NAMESPACE
