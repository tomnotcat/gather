/*
 * Copyright (C) 2013 Tom Wong. All rights reserved.
 */
#include "gtftclient.h"
#include "gtftmessage.pb.h"
#include "gtsvcutil.h"
#include <QtCore/QDebug>
#include <QtCore/qendian.h>
#include <QtNetwork/QHostAddress>
#include <QtNetwork/QTcpSocket>
#include <limits>

GT_BEGIN_NAMESPACE

class GtFTClientPrivate
{
    Q_DECLARE_PUBLIC(GtFTClient)

public:
    explicit GtFTClientPrivate(GtFTClient *q);
    ~GtFTClientPrivate();

public:
    bool open(QIODevice::OpenMode mode);
    void close();
    void disconnect();

protected:
    GtFTClient *q_ptr;
    QTcpSocket *socket;
    QHostAddress address;
    QString session;
    QString fileId;
    int error;
    quint16 port;
    bool opened;
};

GtFTClientPrivate::GtFTClientPrivate(GtFTClient *q)
    : q_ptr(q)
    , error(GtFTClient::NoError)
    , port(0)
    , opened(false)
{
    socket = new QTcpSocket(q);
    q->connect(socket, SIGNAL(error(QAbstractSocket::SocketError)),
               q, SLOT(handleError(QAbstractSocket::SocketError)));
    q->connect(socket, SIGNAL(connected()), q, SLOT(handleConnected()));
    q->connect(socket, SIGNAL(disconnected()), q, SLOT(handleDisconnected()));
}

GtFTClientPrivate::~GtFTClientPrivate()
{
    close();
}

bool GtFTClientPrivate::open(QIODevice::OpenMode mode)
{
    if (opened)
        return false;

    socket->connectToHost(address, port);
    if (!socket->waitForConnected()) {
        error = GtFTClient::ConnectFailed;
        return false;
    }

    GtFTOpenRequest request;
    request.set_session(session.toUtf8().constData());
    request.set_fileid(fileId.toUtf8().constData());
    request.set_mode(mode);

    GtFTOpenResponse response;
    if (!GtSvcUtil::syncRequest<GtFTOpenResponse>(socket,
                                                  GT_FT_OPEN_REQUEST,
                                                  &request,
                                                  GT_FT_OPEN_RESPONSE,
                                                  &response))
    {
        error = GtFTClient::RequestFailed;
        return false;
    }

    error = response.error();
    opened = (GtFTClient::NoError == error);

    return opened;
}

void GtFTClientPrivate::close()
{
    if (!opened)
        return;

    disconnect();
    opened = false;
}

void GtFTClientPrivate::disconnect()
{
    socket->disconnectFromHost();
    if (socket->state() != QAbstractSocket::UnconnectedState)
        socket->waitForDisconnected();
}

GtFTClient::GtFTClient(QObject *parent)
    : QIODevice(parent)
    , d_ptr(new GtFTClientPrivate(this))
{
}

GtFTClient::GtFTClient(const QString &fileId,
                       const QHostAddress &address,
                       quint16 port,
                       const QString &session,
                       QObject *parent)
    : QIODevice(parent)
    , d_ptr(new GtFTClientPrivate(this))
{
    d_ptr->fileId = fileId;
    d_ptr->address = address;
    d_ptr->port = port;
    d_ptr->session = session;
}

GtFTClient::~GtFTClient()
{
}

QString GtFTClient::fileId() const
{
    Q_D(const GtFTClient);
    return d->fileId;
}

void GtFTClient::setFileInfo(const QString &fileId,
                             const QHostAddress &address,
                             quint16 port,
                             const QString &session)
{
    Q_D(GtFTClient);

    d->fileId = fileId;
    d->address = address;
    d->port = port;
    d->session = session;
}

bool GtFTClient::open(OpenMode mode)
{
    Q_D(GtFTClient);

    if (d->open(mode))
        return QIODevice::open(mode);

    if (!d->opened)
        d->disconnect();

    return false;
}

void GtFTClient::close()
{
    Q_D(GtFTClient);

    d->close();
    QIODevice::close();
}

int GtFTClient::error() const
{
    Q_D(const GtFTClient);
    return d->error;
}

void GtFTClient::unsetError()
{
    Q_D(GtFTClient);
    d->error = GtFTClient::NoError;
}

qint64 GtFTClient::size()
{
    return 0;
}

qint64 GtFTClient::readData(char *data, qint64 maxlen)
{
    Q_D(GtFTClient);

    if (!d->opened)
        return -1;

    return 0;
}

qint64 GtFTClient::writeData(const char *data, qint64 len)
{
    Q_D(GtFTClient);

    if (!d->opened) {
        d->error = GtFTClient::InvalidState;
        return -1;
    }

    Q_ASSERT(len + sizeof(quint16) <= std::numeric_limits<quint16>::max());

    QByteArray bytes(sizeof(quint32) + len, -1);
    char *buffer = bytes.data();

    *(quint16*)buffer = qToBigEndian<quint16>(static_cast<quint16>(len + sizeof(quint16)));
    *(quint16*)(buffer + sizeof(quint16)) = qToBigEndian<quint16>(GT_FT_WRITE_REQUEST);

    memcpy(buffer + sizeof(quint32), data, len);
    if (!GtSvcUtil::syncWrite(d->socket, buffer, sizeof(quint32) + len)) {
        d->error = GtFTClient::RequestFailed;
        return -1;
    }

    char rbuf[1024];
    int rlen;

    rlen = GtSvcUtil::readMessage(d->socket, rbuf, sizeof(rbuf));
    if (rlen < (int)sizeof(quint16)) {
        d->error = GtFTClient::RequestFailed;
        return -1;
    }

    if (qFromBigEndian<quint16>(*(quint16*)rbuf) != GT_FT_WRITE_RESPONSE) {
        d->error = GtFTClient::RequestFailed;
        return -1;
    }

    GtFTWriteResponse response;
    if (response.ParseFromArray(rbuf + 2, rlen - 2)) {
        d->error = GtFTClient::RequestFailed;
        return -1;
    }

    return response.size();
}

void GtFTClient::handleConnected()
{
}

void GtFTClient::handleDisconnected()
{
    Q_D(GtFTClient);
    d->opened = false;
}

void GtFTClient::handleError(QAbstractSocket::SocketError error)
{
    Q_D(GtFTClient);

    d->opened = false;

    switch (error) {
    case QAbstractSocket::RemoteHostClosedError:
        d->error = GtFTClient::RemoteClosed;
        break;

    default:
        qWarning() << "GtFTClient socket error:"
                   << error << d->socket->errorString();
        break;
    }
}

GT_END_NAMESPACE
