/*
 * Copyright (C) 2013 Tom Wong. All rights reserved.
 */
#include "gtdocrendercache.h"
#include "gtdocmodel.h"
#include "gtdocpage.h"
#include "gtdocument.h"
#include <QtCore/QDebug>
#include <QtCore/QMutex>
#include <QtGui/QImage>

GT_BEGIN_NAMESPACE

class GtDocRenderCachePrivate
{
    Q_DECLARE_PUBLIC(GtDocRenderCache)

public:
    explicit GtDocRenderCachePrivate(GtDocRenderCache *parent);
    ~GtDocRenderCachePrivate();

public:
    int pageBytes(int index, double scale, int rotation);

    void preloadRange(int beginPage, int endPage,
                      double scale, int rotation,
                      int *preloadBegin, int *preloadEnd);
public:
    enum {
        MaxPreloadedPages = 3
    };

    class CacheInfo {
    public:
        CacheInfo()
            : scale(0.)
            , page(0)
            , rotation(0)
            , rendered(false)
        {
        }

    public:
        QImage image;
        double scale;
        int page;
        int rotation;
        bool rendered;
    };

private:
    GtDocRenderCache *q_ptr;
    GtDocModel *model;
    int maxSize;
    int index;
    int currentPage;
    QVector<CacheInfo> caches;
    QMutex mutex;
};

GtDocRenderCachePrivate::GtDocRenderCachePrivate(GtDocRenderCache *parent)
    : q_ptr(parent)
    , model(NULL)
    , maxSize(0)
    , index(0)
    , currentPage(0)
{
}

GtDocRenderCachePrivate::~GtDocRenderCachePrivate()
{
}

int GtDocRenderCachePrivate::pageBytes(int index, double scale, int rotation)
{
    GtDocument *document = model->document();
    QSize size = document->page(index)->size(scale, rotation);
    return size.width() * size.height() * 4;
}

void GtDocRenderCachePrivate::preloadRange(int beginPage, int endPage,
                                           double scale, int rotation,
                                           int *preloadBegin, int *preloadEnd)
{
    GtDocument *document = model->document();
    int rangeSize = 0;
    int preloadCacheSize = 0;
    int pageCount = document->pageCount();

    /* Get the size of the current range */
    for (int i = beginPage; i < endPage; ++i)
        rangeSize += pageBytes(i, scale, rotation);

    if (rangeSize >= maxSize) {
        *preloadBegin = beginPage;
        *preloadEnd = endPage;
        return;
    }

    int i = 1;
    while (((beginPage - i >= 0) || (endPage + i - 1 < pageCount)) &&
           preloadCacheSize < MaxPreloadedPages)
    {
        int size;
        bool updated = false;

        if (endPage + i - 1 < pageCount) {
            size = pageBytes(endPage + i - 1, scale, rotation);
            if (size + rangeSize <= maxSize) {
                rangeSize += size;
                preloadCacheSize++;
                updated = true;
            }
            else {
                break;
            }
        }

        if (beginPage - i >= 0) {
            size = pageBytes(beginPage - i, scale, rotation);
            if (size + rangeSize <= maxSize) {
                rangeSize += size;
                if (!updated)
                    preloadCacheSize++;
            }
            else {
                break;
            }
        }

        i++;
    }

    *preloadBegin = MAX(beginPage - preloadCacheSize, 0);
    *preloadEnd = MIN(endPage + preloadCacheSize, pageCount);
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
    Q_D(GtDocRenderCache);

    if (model == d->model)
        return;

    if (d->model) {
        disconnect(d->model,
                   SIGNAL(destroyed(QObject*)),
                   this,
                   SLOT(modelDestroyed(QObject*)));
    }

    d->model = model;
    if (d->model) {
        connect(d->model,
                SIGNAL(destroyed(QObject*)),
                this,
                SLOT(modelDestroyed(QObject*)));
    }

    this->clear();
}

void GtDocRenderCache::setMaxSize(int maxSize)
{
    Q_D(GtDocRenderCache);

    if (maxSize != d->maxSize) {
        if (maxSize < d->maxSize)
            this->clear();

        d->maxSize = maxSize;
    }
}

void GtDocRenderCache::setPageRange(int beginPage, int endPage)
{
    Q_D(GtDocRenderCache);

    double scale;
    int rotation;
    int preloadBegin;
    int preloadEnd;
    int i, j, c;

    scale = d->model->scale();
    rotation = d->model->rotation();

    d->preloadRange(beginPage, endPage, scale, rotation,
                    &preloadBegin, &preloadEnd);

    /* Update the cache infos. */
    QMutexLocker lock(&d->mutex);
    int offset = d->index - preloadBegin;
    if (offset > 0) {
        GtDocRenderCachePrivate::CacheInfo *data = d->caches.data();
        c = d->caches.size();
        d->caches.resize(preloadEnd - preloadBegin);

        if (c > d->caches.size() - offset)
            c = d->caches.size() - offset;

        for (i = 0, j = offset; i < c; ++i, ++j) {
            data[j] = data[i];
            data[i] = GtDocRenderCachePrivate::CacheInfo();
        }
    }
    else if (offset < 0) {
        c = -offset;

        if (c > d->caches.size())
            c = d->caches.size();

        d->caches.remove(0, c);
        d->caches.resize(preloadEnd - preloadBegin);
    }
    else {
        d->caches.resize(preloadEnd - preloadBegin);
    }

    d->index = preloadBegin;
    d->currentPage = d->model->page();

    c = d->caches.size();
    for (i = 0; i < c; ++i) {
        GtDocRenderCachePrivate::CacheInfo &info = d->caches[i];

        info.page = d->index + i;
        if (info.scale != scale || info.rotation != rotation) {
            info.scale = scale;
            info.rotation = rotation;
            info.rendered = false;
        }
    }

    QMetaObject::invokeMethod(this, "renderNext", Qt::QueuedConnection);
}

QImage* GtDocRenderCache::image(int index)
{
    Q_D(GtDocRenderCache);

    QMutexLocker lock(&d->mutex);

    if (index < d->index)
        return 0;

    if (index >= d->index + d->caches.size())
        return 0;

    return &d->caches[index].image;
}

void GtDocRenderCache::clear(void)
{
    Q_D(GtDocRenderCache);

    QMutexLocker lock(&d->mutex);
    d->caches.clear();
}

void GtDocRenderCache::modelDestroyed(QObject *object)
{
    Q_D(GtDocRenderCache);

    if (object == static_cast<QObject *>(d->model))
        setModel(0);
}

void GtDocRenderCache::renderNext()
{
    Q_D(GtDocRenderCache);

next:
    double scale = 0;
    int rotation = 0;
    int pageIndex = -1;

    if (1) {
        QMutexLocker lock(&d->mutex);
        int i, j, c;
        GtDocRenderCachePrivate::CacheInfo *info = 0;

        c = d->caches.size();
        i = d->currentPage - d->index;
        i = CLAMP(i, 0, c);
        for (j = i; i > 0 || j < c; --i, ++j) {
            if (i >= 0 && !d->caches[i].rendered) {
                info = &d->caches[i];
                break;
            }

            if (j < c && !d->caches[j].rendered) {
                info = &d->caches[j];
                break;
            }
        }

        if (!info)
            return;

        scale = info->scale;
        rotation = info->rotation;
        pageIndex = info->page;
        info->rendered = true;
    }

    GtDocument *document = d->model->document();
    GtDocPage *page = document->page(pageIndex);
    QSize size = page->size(scale, rotation);
    QImage image(size, QImage::Format_ARGB32);

    page->paint(&image, scale, rotation);

    goto next;
}

GT_END_NAMESPACE
