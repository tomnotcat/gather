/*
 * Copyright (C) 2013 Tom Wong. All rights reserved.
 */
#include "gtftclient.h"
#include "gtftmessage.pb.h"
#include "gtrecvbuffer.h"
#include "gtsvcutil.h"
#include <QtCore/QDebug>
#include <QtCore/qendian.h>
#include <QtNetwork/QHostAddress>
#include <QtNetwork/QTcpSocket>

GT_BEGIN_NAMESPACE

class GtFTClientPrivate
{
    Q_DECLARE_PUBLIC(GtFTClient)

public:
    explicit GtFTClientPrivate(GtFTClient *q);
    ~GtFTClientPrivate();

public:
    bool open(QIODevice::OpenMode mode);
    void close(bool wait);

public:
    void handleMessage(const char *data, int size);
    void handleOpen(GtFTOpenResponse &msg);

protected:
    GtFTClient *q_ptr;
    QTcpSocket *socket;
    GtRecvBuffer buffer;
    QHostAddress address;
    QString session;
    QString fileId;
    qint64 size;
    quint16 port;
    bool connected;
    bool opened;
};

GtFTClientPrivate::GtFTClientPrivate(GtFTClient *q)
    : q_ptr(q)
    , size(0)
    , port(0)
    , connected(false)
    , opened(false)
{
    socket = new QTcpSocket(q);
    q->connect(socket, SIGNAL(readyRead()), q, SLOT(handleRead()));
    q->connect(socket, SIGNAL(error(QAbstractSocket::SocketError)),
               q, SLOT(handleError(QAbstractSocket::SocketError)));
    q->connect(socket, SIGNAL(connected()), q, SLOT(handleConnected()));
    q->connect(socket, SIGNAL(disconnected()), q, SLOT(handleDisconnected()));
}

GtFTClientPrivate::~GtFTClientPrivate()
{
    close(true);
}

bool GtFTClientPrivate::open(QIODevice::OpenMode mode)
{
    Q_UNUSED(mode);

    if (connected) {
        socket->disconnectFromHost();
        if (socket->state() != QAbstractSocket::UnconnectedState)
            socket->waitForDisconnected();
    }

    socket->connectToHost(address, port);
    return true;
}

void GtFTClientPrivate::close(bool wait)
{
    if (!connected)
        return;

    socket->disconnectFromHost();
    if (wait) {
        if (socket->state() != QAbstractSocket::UnconnectedState)
            socket->waitForDisconnected();
    }

    connected = false;
    opened = false;
}

void GtFTClientPrivate::handleMessage(const char *data, int size)
{
    if (size < (int)sizeof(quint16)) {
        qWarning() << "Invalid FT message size:" << size;
        return;
    }

    quint16 type = qFromBigEndian<quint16>(*(quint16*)data);
    data += sizeof(quint16);
    size -= sizeof(quint16);

    switch (type) {
    case GT_FT_OPEN_RESPONSE:
        {
            GtFTOpenResponse msg;
            if (msg.ParseFromArray(data, size)) {
                handleOpen(msg);
            }
            else {
                qWarning() << "Invalid FT open response";
            }
        }
        break;

    default:
        qWarning() << "Invalid FT message type:" << type;
        break;
    }
}

void GtFTClientPrivate::handleOpen(GtFTOpenResponse &msg)
{
    Q_Q(GtFTClient);

    opened = (GtFTClient::OpenSuccess == msg.result());
    size = msg.size();

    emit q->connection(msg.result());
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

    return false;
}

void GtFTClient::close()
{
    Q_D(GtFTClient);
    d->close(false);
}

bool GtFTClient::flush()
{
    Q_D(GtFTClient);
    return d->socket->flush();
}

qint64 GtFTClient::size() const
{
    Q_D(const GtFTClient);
    return d->size;
}

qint64 GtFTClient::readData(char *data, qint64 maxlen)
{
    return 0;
}

qint64 GtFTClient::writeData(const char *data, qint64 len)
{
    return 0;
}

void GtFTClient::handleRead()
{
    Q_D(GtFTClient);

    int result = d->buffer.read(d->socket);
    while (GtRecvBuffer::ReadMessage == result) {
        d->handleMessage(d->buffer.buffer(), d->buffer.size());
        d->buffer.clear();
        result = d->buffer.read(d->socket);
    }

    switch (result) {
    case GtRecvBuffer::ReadError:
        qWarning() << "GtFTClient GtRecvBuffer::ReadError";
        break;

    default:
        break;
    }
}

void GtFTClient::handleConnected()
{
    Q_D(GtFTClient);

    d->connected = true;

    GtFTOpenRequest msg;
    msg.set_session(d->session.toUtf8().constData());
    msg.set_file(d->fileId.toUtf8().constData());
    msg.set_mode(openMode());
    GtSvcUtil::sendMessage(d->socket, GT_FT_OPEN_REQUEST, &msg);
}

void GtFTClient::handleDisconnected()
{
    Q_D(GtFTClient);
    d->connected = false;
    d->opened = false;
}

void GtFTClient::handleError(QAbstractSocket::SocketError error)
{
    Q_D(GtFTClient);

    d->connected = false;
    d->opened = false;

    switch (error) {
    case QAbstractSocket::RemoteHostClosedError:
        break;

    default:
        qWarning() << "GtFTClient socket error:"
                   << error << d->socket->errorString();
        break;
    }
}

GT_END_NAMESPACE
