/*
 * Copyright (C) 2013 Tom Wong. All rights reserved.
 */
#include "gtftupload.h"
#include "gtdocument.h"
#include "gtftmessage.pb.h"
#include "gtftsession.h"
#include "gtsvcutil.h"
#include <QtCore/QCryptographicHash>
#include <QtCore/QDebug>
#include <QtCore/qendian.h>
#include <QtNetwork/QHostAddress>
#include <QtNetwork/QTcpSocket>

GT_BEGIN_NAMESPACE

class GtFTUploadPrivate
{
    Q_DECLARE_PUBLIC(GtFTUpload)

public:
    explicit GtFTUploadPrivate(GtFTUpload *q);
    ~GtFTUploadPrivate();

protected:
    GtFTUpload *q_ptr;
    QHostAddress address;
    QString session;
    QString secret;
    quint16 port;
    QIODevice *device;
};

GtFTUploadPrivate::GtFTUploadPrivate(GtFTUpload *q)
    : q_ptr(q)
{
}

GtFTUploadPrivate::~GtFTUploadPrivate()
{
}

GtFTUpload::GtFTUpload(const QHostAddress &address, quint16 port,
                       const QString &session, const QString &secret,
                       QIODevice *device, QObject *parent)
    : QObject(parent)
    , d_ptr(new GtFTUploadPrivate(this))
{
    d_ptr->address = address;
    d_ptr->port = port;
    d_ptr->session = session;
    d_ptr->secret = secret;
    d_ptr->device = device;
}

GtFTUpload::~GtFTUpload()
{
}

void GtFTUpload::run()
{
    Q_D(GtFTUpload);

    QTcpSocket tcpSocket;

    tcpSocket.connectToHost(d->address, d->port);
    if (!tcpSocket.waitForConnected()) {
        emit error(ConnectionError);
        return;
    }

    if (!initUpload(&tcpSocket))
        return;

    return;
    char buffer[10240];
    qint64 offset = 0;
    qint64 length = d->device->size();
    qint64 bytesRead;
    qint64 bytesWrite;
    qint64 curWrite;

    if (!d->device->seek(offset)) {
        // invalid offset
        return;
    }

    if (offset + length > d->device->size()) {
        // invalid block
        return;
    }

    while (length > 0) {
        bytesRead = sizeof(buffer) < length ? sizeof(buffer) : length;
        bytesRead = d->device->read(buffer, bytesRead);

        if (bytesRead <= 0) {
            // read error
            break;
        }

        bytesWrite = 0;
        while (bytesWrite < bytesRead) {
            curWrite = tcpSocket.write(buffer, bytesRead - bytesWrite);

            if (curWrite <= 0) {
                // write error
                break;
            }

            if (!tcpSocket.waitForBytesWritten()) {
                // send error
                break;
            }

            bytesWrite += curWrite;
        }

        length -= bytesRead;
    }
}

bool GtFTUpload::initUpload(QAbstractSocket *socket)
{
    Q_D(GtFTUpload);

    GtFTUploadRequest request;
    QCryptographicHash hash(QCryptographicHash::Md5);
    qint64 temp = d->device->size();

    request.set_session(d->session.toUtf8().constData());
    request.set_fileid(GtDocument::makeDocId(d->device).toHex());
    request.set_filesize(temp);

    hash.addData(request.session().c_str(), request.session().length());
    hash.addData(request.fileid().c_str(), request.fileid().length());

    temp = qFromBigEndian<qint64>(temp);
    hash.addData((const char*)&temp, sizeof(temp));
    request.set_signature(hash.result().toHex());

    GtSvcUtil::sendMessage(socket, GT_UPLOAD_REQUEST, &request);
    if (!socket->waitForBytesWritten()) {
        emit error(SendError);
        return false;
    }

    char buffer[1024];
    int length;

    length = GtSvcUtil::readMessage(socket, buffer, sizeof(buffer));
    if (length < (int)sizeof(quint16)) {
        emit error(DataError);
        return false;
    }

    if (qFromBigEndian<quint16>(*(quint16*)buffer) != GT_UPLOAD_RESPONSE) {
        emit error(DataError);
        return false;
    }

    GtFTUploadResponse response;
    if (!response.ParseFromArray(buffer + 2, length - 2)) {
        emit error(DataError);
        return false;
    }

    if (response.result() != GtFTSession::AuthSuccess) {
        emit error(AuthError);
        return false;
    }

    return true;
}

GT_END_NAMESPACE
