/*
 * Copyright (C) 2013 Tom Wong. All rights reserved.
 */
#include "gtbookmarks.h"
#include "gtbookmark.h"
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

GT_END_NAMESPACE
