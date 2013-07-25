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
    GtBookmark root;
};

GtBookmarksPrivate::GtBookmarksPrivate(GtBookmarks *q)
    : q_ptr(q)
{
}

GtBookmarksPrivate::~GtBookmarksPrivate()
{
}

GtBookmarks::GtBookmarks(QObject *parent)
    : QObject(parent)
    , d_ptr(new GtBookmarksPrivate(this))
{
}

GtBookmarks::~GtBookmarks()
{
}

const GtBookmark* GtBookmarks::root() const
{
    Q_D(const GtBookmarks);
    return &d->root;
}

GtBookmark* GtBookmarks::root()
{
    Q_D(GtBookmarks);
    return &d->root;
}

GT_END_NAMESPACE
