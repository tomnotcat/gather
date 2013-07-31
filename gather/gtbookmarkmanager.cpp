/*
 * Copyright (C) 2013 Tom Wong. All rights reserved.
 */
#include "gtbookmarkmanager.h"
#include <QtCore/QDebug>

GT_BEGIN_NAMESPACE

class GtBookmarkManagerPrivate
{
    Q_DECLARE_PUBLIC(GtBookmarkManager)

public:
    GtBookmarkManagerPrivate(GtBookmarkManager *q);
    ~GtBookmarkManagerPrivate();

protected:
    GtBookmarkManager *q_ptr;
};

GtBookmarkManagerPrivate::GtBookmarkManagerPrivate(GtBookmarkManager *q)
    : q_ptr(q)
{
}

GtBookmarkManagerPrivate::~GtBookmarkManagerPrivate()
{
}

GtBookmarkManager::GtBookmarkManager(QObject *parent)
    : QObject(parent)
    , d_ptr(new GtBookmarkManagerPrivate(this))
{
}

GtBookmarkManager::~GtBookmarkManager()
{
}

GtBookmarks *GtBookmarkManager::loadBookmarks(const QString &id)
{
    return 0;
}

GT_END_NAMESPACE
