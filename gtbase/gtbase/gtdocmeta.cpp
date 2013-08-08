/*
 * Copyright (C) 2013 Tom Wong. All rights reserved.
 */
#include "gtdocmeta.h"
#include "gtdocmessage.pb.h"
#include "gtserialize.h"
#include <QtCore/QDebug>

GT_BEGIN_NAMESPACE

class GtDocMetaPrivate
{
    Q_DECLARE_PUBLIC(GtDocMeta)

public:
    GtDocMetaPrivate(GtDocMeta *q);
    ~GtDocMetaPrivate();

protected:
    GtDocMeta *q_ptr;
    QString m_id;
    QString m_bookmarksId;
    QString m_notesId;
};

GtDocMetaPrivate::GtDocMetaPrivate(GtDocMeta *q)
    : q_ptr(q)
{
}

GtDocMetaPrivate::~GtDocMetaPrivate()
{
}

GtDocMeta::GtDocMeta(const QString &id, QObject *parent)
    : QObject(parent)
    , d_ptr(new GtDocMetaPrivate(this))
{
    d_ptr->m_id = id;
}

GtDocMeta::~GtDocMeta()
{
}

QString GtDocMeta::id() const
{
    return d_ptr->m_id;
}

QString GtDocMeta::bookmarksId() const
{
    return d_ptr->m_bookmarksId;
}

void GtDocMeta::setBookmarksId(const QString &id)
{
    Q_D(GtDocMeta);

    if (d->m_bookmarksId != id) {
        d->m_bookmarksId = id;
        emit bookmarksIdChanged(id);
    }
}

QString GtDocMeta::notesId() const
{
    return d_ptr->m_notesId;
}

void GtDocMeta::setNotesId(const QString &id)
{
    Q_D(GtDocMeta);

    if (d->m_notesId != id) {
        d_ptr->m_notesId = id;
        emit notesIdChanged(id);
    }
}

void GtDocMeta::serialize(GtDocMetaMsg &msg) const
{
    Q_D(const GtDocMeta);

    msg.set_id(d->m_id.toUtf8());
    msg.set_bookmarks_id(d->m_bookmarksId.toUtf8());
    msg.set_notes_id(d->m_notesId.toUtf8());
}

bool GtDocMeta::deserialize(const GtDocMetaMsg &msg)
{
    Q_D(GtDocMeta);

    if (d->m_id != msg.id().c_str())
        return false;

    setBookmarksId(msg.bookmarks_id().c_str());
    setNotesId(msg.notes_id().c_str());
    return true;
}

#ifndef QT_NO_DATASTREAM

QDataStream &operator<<(QDataStream &s, const GtDocMeta &m)
{
    return GtSerialize::serialize<GtDocMeta, GtDocMetaMsg>(s, m);
}

QDataStream &operator>>(QDataStream &s, GtDocMeta &m)
{
    return GtSerialize::deserialize<GtDocMeta, GtDocMetaMsg>(s, m);
}

#endif // QT_NO_DATASTREAM

GT_END_NAMESPACE
