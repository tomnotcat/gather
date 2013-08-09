/*
 * Copyright (C) 2013 Tom Wong. All rights reserved.
 */
#include "gtuserclient.h"
#include "gtrecvbuffer.h"
#include "gtsvcutil.h"
#include "gtusermessage.pb.h"
#include <QtCore/QDebug>
#include <QtCore/qendian.h>
#include <QtNetwork/QTcpSocket>

GT_BEGIN_NAMESPACE

class GtUserClientPrivate
{
    Q_DECLARE_PUBLIC(GtUserClient)

public:
    explicit GtUserClientPrivate(GtUserClient *q);
    ~GtUserClientPrivate();

public:
    void handleMessage(const char *data, int size);
    void handleLogin(GtUserLoginResponse &msg);

protected:
    GtUserClient *q_ptr;
    QTcpSocket m_socket;
    GtRecvBuffer m_buffer;
    GtUserClient::ErrorCode m_error;
    GtUserClient::StateCode m_state;
};

GtUserClientPrivate::GtUserClientPrivate(GtUserClient *q)
    : q_ptr(q)
    , m_error(GtUserClient::ErrorNone)
    , m_state(GtUserClient::UnconnectedState)
{
    QObject::connect(&m_socket, SIGNAL(readyRead()),
                     q, SLOT(handleRead()));

    QObject::connect(&m_socket, SIGNAL(error(QAbstractSocket::SocketError)),
               q, SLOT(handleError(QAbstractSocket::SocketError)));

    QObject::connect(&m_socket, SIGNAL(connected()),
                     q, SLOT(handleConnected()));

    QObject::connect(&m_socket, SIGNAL(disconnected()),
                     q, SLOT(handleDisconnected()));
}

GtUserClientPrivate::~GtUserClientPrivate()
{
}

void GtUserClientPrivate::handleMessage(const char *data, int size)
{
    if (size < (int)sizeof(quint16)) {
        qWarning() << "Invalid user message size:" << size;
        return;
    }

    quint16 type = qFromBigEndian<quint16>(*(quint16*)data);
    data += sizeof(quint16);
    size -= sizeof(quint16);

    switch (type) {
    case GT_USER_LOGIN_RESPONSE:
        {
            GtUserLoginResponse msg;
            if (msg.ParseFromArray(data, size)) {
                handleLogin(msg);
            }
            else {
                qWarning() << "Invalid user login response";
            }
        }
        break;

    case GT_USER_LOGOUT_RESPONSE:
        {
            GtUserLogoutResponse msg;
            if (msg.ParseFromArray(data, size)) {
                Q_ASSERT(0);
            }
            else {
                qWarning() << "Invalid user logout response";
            }
        }
        break;

    default:
        qWarning() << "Invalid user message type:" << type;
        break;
    }
}

GtUserClient::GtUserClient(QObject *parent)
    : QObject(parent)
    , d_ptr(new GtUserClientPrivate(this))
{
}

GtUserClient::~GtUserClient()
{
}

bool GtUserClient::connect(const QHostAddress &address, quint16 port)
{
    Q_D(GtUserClient);

    if (d->m_state != UnconnectedState) {
        d->m_error = ErrorInvalidState;
        return false;
    }

    d->m_socket.connectToHost(address, port);
    return d->m_socket.waitForConnected();
}

void GtUserClient::disconnect()
{
    Q_D(GtUserClient);

    if (d->m_state == UnconnectedState)
        return;

    d->m_socket.disconnectFromHost();
    if (d->m_socket.state() != QAbstractSocket::UnconnectedState)
        d->m_socket.waitForDisconnected();
}

bool GtUserClient::login(const QString &user, const QString &passwd)
{
    Q_D(GtUserClient);

    if (d->m_state != ConnectedState) {
        d->m_error = ErrorInvalidState;
        return false;
    }

    GtUserLoginRequest msg;

    msg.set_user(user.toUtf8());
    msg.set_passwd(passwd.toUtf8());

    if (!GtSvcUtil::sendMessage(&d->m_socket, GT_USER_LOGIN_REQUEST, &msg))
        return false;

    return true;
}

void GtUserClient::logout()
{
    Q_D(GtUserClient);

    if (d->m_state != LoggedInState)
        return;

    GtUserLogoutRequest msg;
    GtSvcUtil::sendMessage(&d->m_socket, GT_USER_LOGOUT_REQUEST, &msg);

    d->m_state = ConnectedState;
}

void GtUserClient::handleRead()
{
    Q_D(GtUserClient);

    int result = d->m_buffer.read(&d->m_socket);
    while (GtRecvBuffer::ReadMessage == result) {
        d->handleMessage(d->m_buffer.buffer(), d->m_buffer.size());
        d->m_buffer.clear();
        result = d->m_buffer.read(&d->m_socket);
    }

    switch (result) {
    case GtRecvBuffer::ReadError:
        qWarning() << "GtUserClient GtRecvBuffer::ReadError";
        break;

    default:
        break;
    }
}

void GtUserClient::handleConnected()
{
    Q_D(GtUserClient);
    d->m_state = ConnectedState;
}

void GtUserClient::handleDisconnected()
{
    Q_D(GtUserClient);
    d->m_state = UnconnectedState;
}

void GtUserClient::handleError(QAbstractSocket::SocketError error)
{
    Q_D(GtUserClient);

    switch (error) {
    case QAbstractSocket::RemoteHostClosedError:
        break;

    default:
        qWarning() << "GtUserClient socket error:"
                   << error << d->m_socket.errorString();
        break;
    }
}

GT_END_NAMESPACE
