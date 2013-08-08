/*
 * Copyright (C) 2013 Tom Wong. All rights reserved.
 */
#include "gtbookmarks.h"
#include "gtbookmark.h"
#include "gtdocmessage.pb.h"
#include "gtserialize.h"
#include <QtCore/QDebug>

GT_BEGIN_NAMESPACE

class GtBookmarksPrivate
{
    Q_DECLARE_PUBLIC(GtBookmarks)

public:
    explicit GtBookmarksPrivate(GtBookmarks *q);
    ~GtBookmarksPrivate();

protected:
    GtBookmarks *q_ptr;
    QString m_id;
    GtBookmark m_root;
};

GtBookmarksPrivate::GtBookmarksPrivate(GtBookmarks *q)
    : q_ptr(q)
{
}

GtBookmarksPrivate::~GtBookmarksPrivate()
{
}

GtBookmarks::GtBookmarks(const QString &id, QObject *parent)
    : QObject(parent)
    , d_ptr(new GtBookmarksPrivate(this))
{
    d_ptr->m_id = id;
}

GtBookmarks::~GtBookmarks()
{
}

QString GtBookmarks::id() const
{
    Q_D(const GtBookmarks);
    return d->m_id;
}

const GtBookmark* GtBookmarks::root() const
{
    Q_D(const GtBookmarks);
    return &d->m_root;
}

GtBookmark* GtBookmarks::root()
{
    Q_D(GtBookmarks);
    return &d->m_root;
}

void GtBookmarks::serialize(GtBookmarksMsg &msg) const
{
    Q_D(const GtBookmarks);

    msg.set_id(d->m_id.toUtf8());
    d->m_root.serialize(*msg.mutable_root());
}

bool GtBookmarks::deserialize(const GtBookmarksMsg &msg)
{
    Q_D(GtBookmarks);

    if (d->m_id != msg.id().c_str())
        return false;

    return d->m_root.deserialize(msg.root());
}

#ifndef QT_NO_DATASTREAM

QDataStream &operator<<(QDataStream &s, const GtBookmarks &b)
{
    return GtSerialize::serialize<GtBookmarks, GtBookmarksMsg>(s, b);
}

QDataStream &operator>>(QDataStream &s, GtBookmarks &b)
{
    return GtSerialize::deserialize<GtBookmarks, GtBookmarksMsg>(s, b);
}

#endif // QT_NO_DATASTREAM

GT_END_NAMESPACE
