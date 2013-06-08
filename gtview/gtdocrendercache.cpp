/*
 * Copyright (C) 2013 Tom Wong. All rights reserved.
 */
#include "gtdocrendercache.h"
#include <QtCore/QDebug>

GT_BEGIN_NAMESPACE

class GtDocRenderCachePrivate
{
    Q_DECLARE_PUBLIC(GtDocRenderCache)

public:
    explicit GtDocRenderCachePrivate(GtDocRenderCache *parent);
    ~GtDocRenderCachePrivate();

private:
    GtDocRenderCache *q_ptr;
    GtDocModel *model;
};

GtDocRenderCachePrivate::GtDocRenderCachePrivate(GtDocRenderCache *parent)
    : q_ptr(parent)
    , model(NULL)

{
}

GtDocRenderCachePrivate::~GtDocRenderCachePrivate()
{
}

GtDocRenderCache::GtDocRenderCache(QObject *parent)
    : QObject(parent)
    , d_ptr(new GtDocRenderCachePrivate(this))
{
}

GtDocRenderCache::~GtDocRenderCache()
{
}

void GtDocRenderCache::setModel(GtDocModel *model)
{
}

void GtDocRenderCache::setMaxSize(int maxSize)
{
}

void GtDocRenderCache::setPageRange(int beginPage, int endPage)
{
}

QImage* GtDocRenderCache::pageImage(int index)
{
    return 0;
}

void GtDocRenderCache::clear(void)
{
}

GT_END_NAMESPACE
