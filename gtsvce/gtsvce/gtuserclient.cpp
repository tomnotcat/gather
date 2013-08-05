/*
 * Copyright (C) 2013 Tom Wong. All rights reserved.
 */
#include "gtuserclient.h"
#include "gtbookmark.h"
#include "gtbookmarks.h"
#include "gtdocmeta.h"
#include "gtdocnote.h"
#include "gtdocnotes.h"
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
    void sendLogin() const;

public:
    void handleMessage(const char *data, int size);
    void handleLogin(GtUserLoginResponse &msg);

public:
    static void convert(const GtDocRange &s, GtUserDocRange &d);
    static void convert(const GtUserDocRange &s, GtDocRange &d);

    static void convert(const GtLinkDest &s, GtUserLinkDest &d);
    static void convert(const GtUserLinkDest &s, GtLinkDest &d);

    static void convert(const GtBookmark *s, GtUserBookmark *d);
    static void convert(const GtUserBookmark *s, GtBookmark *d);

protected:
    GtUserClient *q_ptr;
    QTcpSocket *socket;
    GtRecvBuffer buffer;
    QString user;
    QString passwd;
    bool connected;
    bool logined;
};

GtUserClientPrivate::GtUserClientPrivate(GtUserClient *q)
    : q_ptr(q)
    , connected(false)
    , logined(false)
{
    socket = new QTcpSocket(q);
    q->connect(socket, SIGNAL(readyRead()), q, SLOT(handleRead()));
    q->connect(socket, SIGNAL(error(QAbstractSocket::SocketError)),
               q, SLOT(handleError(QAbstractSocket::SocketError)));
    q->connect(socket, SIGNAL(connected()), q, SLOT(handleConnected()));
    q->connect(socket, SIGNAL(disconnected()), q, SLOT(handleDisconnected()));
}

GtUserClientPrivate::~GtUserClientPrivate()
{
    if (connected) {
        socket->disconnectFromHost();
        if (socket->state() != QAbstractSocket::UnconnectedState)
            socket->waitForDisconnected();
    }
}

void GtUserClientPrivate::sendLogin() const
{
    GtUserLoginRequest msg;

    msg.set_user(user.toUtf8().constData());
    msg.set_passwd(passwd.toUtf8().constData());

    GtSvcUtil::sendMessage(socket, GT_USER_LOGIN_REQUEST, &msg);
}

void GtUserClientPrivate::handleMessage(const char *data, int size)
{
    Q_Q(GtUserClient);

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
                emit q->logout(msg.reason());
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

void GtUserClientPrivate::handleLogin(GtUserLoginResponse &msg)
{
    Q_Q(GtUserClient);

    logined = (GtUserClient::LoginSuccess == msg.result());

    emit q->login(msg.result());
}

void GtUserClientPrivate::convert(const GtDocRange &s, GtUserDocRange &d)
{
    GtDocPoint b(s.begin());
    GtDocPoint e(s.end());

    d.set_type(s.type());
    d.set_begin_page(b.page());
    d.set_begin_x(b.x());
    d.set_begin_y(b.y());
    d.set_end_page(e.page());
    d.set_end_x(e.x());
    d.set_end_y(e.y());
}

void GtUserClientPrivate::convert(const GtUserDocRange &s, GtDocRange &d)
{
    GtDocPoint b(s.begin_page(), s.begin_x(), s.begin_y());
    GtDocPoint e(s.end_page(), s.end_x(), s.end_y());

    switch (s.type()) {
    case GtDocRange::TextRange:
    case GtDocRange::GeomRange:
        d.setType((GtDocRange::RangeType)s.type());
        break;

    default:
        qWarning() << "invalid doc range type:" << s.type();
        break;
    }

    d.setRange(b, e);
}

void GtUserClientPrivate::convert(const GtLinkDest &s, GtUserLinkDest &d)
{
    switch (s.type()) {
    case GtLinkDest::ScrollTo:
        d.set_type(s.type());
        d.set_page(s.page());
        d.set_x(s.point().x());
        d.set_y(s.point().y());
        d.set_zoom(s.zoom());
        break;

    case GtLinkDest::LaunchURI:
        d.set_type(s.type());
        d.set_uri(s.uri().toUtf8());
        break;

    default:
        break;
    }
}

void GtUserClientPrivate::convert(const GtUserLinkDest &s, GtLinkDest &d)
{
    switch (s.type()) {
    case GtLinkDest::ScrollTo:
        d.setScrollTo(s.page(), QPointF(s.x(), s.y()), s.zoom());
        break;

    case GtLinkDest::LaunchURI:
        d.setLaunchUri(s.uri().c_str());
        break;

    default:
        break;
    }
}

void GtUserClientPrivate::convert(const GtBookmark *s, GtUserBookmark *d)
{
    d->set_title(s->title().toUtf8());
    convert(s->dest(), *d->mutable_dest());

    QList<GtBookmark*> l = s->children();
    QList<GtBookmark*>::iterator i;

    for (i = l.begin(); i != l.end(); ++i) {
        convert(*i, d->add_children());
    }
}

void GtUserClientPrivate::convert(const GtUserBookmark *s, GtBookmark *d)
{
    GtLinkDest dest;

    d->setTitle(s->title().c_str());
    convert(s->dest(), dest);
    d->setDest(dest);

    int count = s->children_size();
    for (int i = 0; i < count; ++i) {
        GtBookmark *b = new GtBookmark();
        convert(&s->children(i), b);
        d->append(b);
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

void GtUserClient::login(const QHostAddress &address, quint16 port,
                         const QString &user, const QString &passwd)
{
    Q_D(GtUserClient);

    d->user = user;
    d->passwd = passwd;

    if (d->connected) {
        d->sendLogin();
    }
    else {
        d->socket->connectToHost(address, port);
    }
}

void GtUserClient::logout()
{
    Q_D(GtUserClient);

    if (d->connected) {
        d->socket->disconnectFromHost();
        d->connected = false;
        d->logined = false;
    }

    emit logout(LogoutNormal);
}

bool GtUserClient::convert(const GtDocMeta &src, GtUserDocMeta &dest)
{
    dest.set_id(src.id().toUtf8());
    dest.set_bookmarks_id(src.bookmarksId().toUtf8());
    dest.set_notes_id(src.notesId().toUtf8());
    return true;
}

bool GtUserClient::convert(const GtUserDocMeta &src, GtDocMeta &dest)
{
    if (strcmp(dest.id().toUtf8().constData(), src.id().c_str()))
        return false;

    dest.setBookmarksId(src.bookmarks_id().c_str());
    dest.setNotesId(src.notes_id().c_str());
    return true;
}

bool GtUserClient::convert(const GtBookmarks &src, GtUserBookmarks &dest)
{
    dest.set_id(src.id().toUtf8());
    GtUserClientPrivate::convert(src.root(), dest.mutable_root());
    return true;
}

bool GtUserClient::convert(const GtUserBookmarks &src, GtBookmarks &dest)
{
    if (strcmp(dest.id().toUtf8().constData(), src.id().c_str()))
        return false;

    GtUserClientPrivate::convert(&src.root(), dest.root());
    return true;
}

bool GtUserClient::convert(const GtDocNotes &src, GtUserDocNotes &dest)
{
    dest.set_id(src.id().toUtf8());

    QList<GtDocNote*> l = src.allNotes();
    QList<GtDocNote*>::iterator i;

    for (i = l.begin(); i != l.end(); ++i) {
        GtDocNote *n = *i;
        GtUserDocNote *p = dest.add_notes();

        switch (n->type()) {
        case GtDocNote::Highlight:
        case GtDocNote::Underline:
            p->set_type(n->type());
            break;

        default:
            qWarning() << "invalid doc note type:" << n->type();
            break;
        }

        GtUserClientPrivate::convert(n->range(), *p->mutable_range());
    }

    return true;
}

bool GtUserClient::convert(const GtUserDocNotes &src, GtDocNotes &dest)
{
    if (strcmp(dest.id().toUtf8().constData(), src.id().c_str()))
        return false;

    int count = src.notes_size();
    for (int i = 0; i < count; ++i) {
        const GtUserDocNote &p = src.notes(i);
        GtDocNote::NoteType type = GtDocNote::NullNote;
        GtDocRange range;

        switch (p.type()) {
        case GtDocNote::Highlight:
        case GtDocNote::Underline:
            type = (GtDocNote::NoteType)p.type();
            break;

        default:
            qWarning() << "invalid doc note type:" << p.type();
            break;
        }

        GtUserClientPrivate::convert(p.range(), range);
        dest.addNote(new GtDocNote(type, range));
    }

    return true;
}

void GtUserClient::handleRead()
{
    Q_D(GtUserClient);

    int result = d->buffer.read(d->socket);
    while (GtRecvBuffer::ReadMessage == result) {
        d->handleMessage(d->buffer.buffer(), d->buffer.size());
        d->buffer.clear();
        result = d->buffer.read(d->socket);
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
    d->connected = true;
    d->sendLogin();
}

void GtUserClient::handleDisconnected()
{
    Q_D(GtUserClient);
    d->connected = false;
    d->logined = false;
}

void GtUserClient::handleError(QAbstractSocket::SocketError error)
{
    Q_D(GtUserClient);

    d->connected = false;
    d->logined = false;

    switch (error) {
    case QAbstractSocket::RemoteHostClosedError:
        break;

    default:
        qWarning() << "GtUserClient socket error:"
                   << error << d->socket->errorString();
        break;
    }
}

GT_END_NAMESPACE
