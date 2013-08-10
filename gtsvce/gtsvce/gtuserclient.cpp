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
    bool sendMessage(int type, const ::google::protobuf::Message *msg);
    int nextMessage(const char **data, int *size, bool wait);
    void handleMessage(int type, const char *data, int size);
    void disconnect();

protected:
    GtUserClient *q_ptr;
    QTcpSocket m_socket;
    GtRecvBuffer m_buffer;
    QString m_sessionId;
    int m_receiving;
    GtUserClient::ErrorCode m_error;
    GtUserClient::StateCode m_state;
};

GtUserClientPrivate::GtUserClientPrivate(GtUserClient *q)
    : q_ptr(q)
    , m_receiving(0)
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
    disconnect();
}

bool GtUserClientPrivate::sendMessage(int type, const ::google::protobuf::Message *msg)
{
    if (GtSvcUtil::sendMessage(&m_socket, type, msg))
        return true;

    m_error = GtUserClient::ErrorSendFail;
    disconnect();
    return false;
}

int GtUserClientPrivate::nextMessage(const char **data, int *size, bool wait)
{
    m_receiving++;
    int result = m_buffer.read(&m_socket, wait);
    m_receiving--;

    if (result == GtRecvBuffer::ReadError) {
        qWarning() << "Receive user message error";

        m_error = GtUserClient::ErrorReceiveFail;
        disconnect();
        return 0;
    }

    if (result != GtRecvBuffer::ReadMessage)
        return 0;

    const char *p = m_buffer.buffer();
    int n = m_buffer.size();
    if (n < (int)sizeof(quint16)) {
        qWarning() << "Invalid user message size:" << n;

        m_error = GtUserClient::ErrorInvalidMessage;
        disconnect();
        return 0;
    }

    // NOTE: just reset the cursor, the data in the buffer is not changed
    m_buffer.clear();

    *data = p + sizeof(quint16);
    *size = n - sizeof(quint16);

    return qFromBigEndian<quint16>(*(quint16*)p);
}

void GtUserClientPrivate::handleMessage(int type, const char *data, int size)
{
    Q_UNUSED(data);
    Q_UNUSED(size);

    switch (type) {
    default:
        qWarning() << "Unknown user message type:" << type;
        break;
    }
}

void GtUserClientPrivate::disconnect()
{
    if (m_state == GtUserClient::UnconnectedState)
        return;

    m_socket.disconnectFromHost();
    if (m_socket.state() != QAbstractSocket::UnconnectedState)
        m_socket.waitForDisconnected();
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
    d->disconnect();
}

bool GtUserClient::login(const QString &user, const QString &passwd)
{
    Q_D(GtUserClient);

    if (d->m_state != ConnectedState) {
        d->m_error = ErrorInvalidState;
        return false;
    }

    GtUserLoginRequest request;

    request.set_user(user.toUtf8());
    request.set_passwd(passwd.toUtf8());

    if (!d->sendMessage(GT_USER_LOGIN_REQUEST, &request))
        return false;

    const char *data;
    int size;
    int type = d->nextMessage(&data, &size, true);
    while (type && type != GT_USER_LOGIN_RESPONSE) {
        d->handleMessage(type, data, size);
        type = d->nextMessage(&data, &size, true);
    }

    if (!type)
        return false;

    GtUserLoginResponse response;
    if (!response.ParseFromArray(data, size)) {
        qWarning() << "Invalid user login response";
        return false;
    }

    bool success = true;
    if (response.has_error()) {
        d->m_error = (ErrorCode)response.error();
        success = (d->m_error == GtUserClient::ErrorNone);
    }

    if (success) {
        d->m_sessionId = response.session_id().c_str();
        d->m_state = LoggedInState;
    }

    return success;
}

void GtUserClient::logout()
{
    Q_D(GtUserClient);

    if (d->m_state != LoggedInState)
        return;

    GtUserLogoutRequest msg;
    d->sendMessage(GT_USER_LOGOUT_REQUEST, &msg);

    d->m_state = ConnectedState;
    d->m_sessionId.clear();
}

QString GtUserClient::sessionId() const
{
    Q_D(const GtUserClient);
    return d->m_sessionId;
}

GtUserClient::ErrorCode GtUserClient::error() const
{
    Q_D(const GtUserClient);
    return d->m_error;
}

GtUserClient::StateCode GtUserClient::state() const
{
    Q_D(const GtUserClient);
    return d->m_state;
}

void GtUserClient::clearError()
{
    Q_D(GtUserClient);
    d->m_error = ErrorNone;
}

void GtUserClient::handleRead()
{
    Q_D(GtUserClient);

    if (d->m_receiving > 0)
        return;

    const char *data;
    int size;
    int type = d->nextMessage(&data, &size, false);

    if (type)
        d->handleMessage(type, data, size);
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
    d->m_sessionId.clear();
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
