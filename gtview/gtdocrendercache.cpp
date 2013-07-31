/*
 * Copyright (C) 2013 Tom Wong. All rights reserved.
 */
#include "gtdocrendercache.h"
#include "gtdocmodel.h"
#include "gtdocpage.h"
#include "gtdocview.h"
#include "gtdocument.h"
#include <QtCore/QDebug>
#include <QtCore/QMutex>
#include <QtCore/QThread>
#include <QtGui/QColor>
#include <QtGui/QImage>

GT_BEGIN_NAMESPACE

class GtDocRenderCachePrivate
{
    Q_DECLARE_PUBLIC(GtDocRenderCache)

public:
    explicit GtDocRenderCachePrivate(GtDocRenderCache *parent);
    ~GtDocRenderCachePrivate();

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

public:
    int pageBytes(int index, double scale, int rotation);

    void preloadRange(int beginPage, int endPage,
                      double scale, int rotation,
                      int *preloadBegin, int *preloadEnd);

    inline CacheInfo* cacheInfo(int i) {
        if (i >= m_index && i < m_index + m_caches.size())
            return &m_caches[i - m_index];

        return 0;
    }

private:
    GtDocRenderCache *q_ptr;
    GtDocView *m_view;
    int m_maxSize;
    int m_index;
    int m_currentPage;
    QVector<CacheInfo> m_caches;
    QMutex m_mutex;
};

GtDocRenderCachePrivate::GtDocRenderCachePrivate(GtDocRenderCache *parent)
    : q_ptr(parent)
    , m_view(0)
    , m_maxSize(0)
    , m_index(0)
    , m_currentPage(0)
{
}

GtDocRenderCachePrivate::~GtDocRenderCachePrivate()
{
}

int GtDocRenderCachePrivate::pageBytes(int index, double scale, int rotation)
{
    GtDocModel *model = m_view->model();
    GtDocument *document = model->document();
    QSize size = document->page(index)->size(scale, rotation);
    return size.width() * size.height() * 4;
}

void GtDocRenderCachePrivate::preloadRange(int beginPage, int endPage,
                                           double scale, int rotation,
                                           int *preloadBegin, int *preloadEnd)
{
    GtDocModel *model = m_view->model();
    GtDocument *document = model->document();
    int rangeSize = 0;
    int preloadCacheSize = 0;
    int pageCount = document->pageCount();

    /* Get the size of the current range */
    for (int i = beginPage; i < endPage; ++i)
        rangeSize += pageBytes(i, scale, rotation);

    if (rangeSize >= m_maxSize) {
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
            if (size + rangeSize <= m_maxSize) {
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
            if (size + rangeSize <= m_maxSize) {
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

GtDocRenderCache::GtDocRenderCache(GtDocView *view, QObject *parent)
    : QObject(parent)
    , d_ptr(new GtDocRenderCachePrivate(this))
{
    d_ptr->m_view = view;
}

GtDocRenderCache::~GtDocRenderCache()
{
}

void GtDocRenderCache::setMaxSize(int maxSize)
{
    Q_D(GtDocRenderCache);

    if (maxSize != d->m_maxSize) {
        if (maxSize < d->m_maxSize)
            clear();

        d->m_maxSize = maxSize;
    }
}

void GtDocRenderCache::setPageRange(int beginPage, int endPage, int currentPage)
{
    Q_D(GtDocRenderCache);

    double scale;
    int rotation;
    int preloadBegin;
    int preloadEnd;
    int i, c;

    if (863 == beginPage) {
        i = 0;
        c = 0;
    }

    scale = d->m_view->scale();
    rotation = d->m_view->rotation();

    d->preloadRange(beginPage, endPage, scale, rotation,
                    &preloadBegin, &preloadEnd);

    /* Update the cache infos. */
    QMutexLocker lock(&d->m_mutex);
    int offset = d->m_index - preloadBegin;
    if (offset > 0) {
        d->m_caches.resize(preloadEnd - preloadBegin);
        c = d->m_caches.size();

        GtDocRenderCachePrivate::CacheInfo *data = d->m_caches.data();
        for (i = c - 1; i >= 0; --i) {
            if (i >= offset) {
                data[i] = data[i - offset];
            }
            else {
                data[i] = GtDocRenderCachePrivate::CacheInfo();
            }
        }
    }
    else if (offset < 0) {
        c = -offset;

        if (c > d->m_caches.size())
            c = d->m_caches.size();

        d->m_caches.remove(0, c);
        d->m_caches.resize(preloadEnd - preloadBegin);
    }
    else {
        d->m_caches.resize(preloadEnd - preloadBegin);
    }

    d->m_index = preloadBegin;
    d->m_currentPage = currentPage;

    c = d->m_caches.size();
    for (i = 0; i < c; ++i) {
        GtDocRenderCachePrivate::CacheInfo &info = d->m_caches[i];

        info.page = d->m_index + i;
        if (info.scale != scale || info.rotation != rotation) {
            info.scale = scale;
            info.rotation = rotation;
            info.rendered = false;
        }
    }

    QMetaObject::invokeMethod(this, "renderNext", Qt::QueuedConnection);
}

QImage GtDocRenderCache::image(int index)
{
    Q_D(GtDocRenderCache);

    QImage image;
    QMutexLocker lock(&d->m_mutex);

    GtDocRenderCachePrivate::CacheInfo *info = d->cacheInfo(index);
    if (info)
        image = info->image;

    return image;
}

void GtDocRenderCache::clear()
{
    Q_D(GtDocRenderCache);

    QMutexLocker lock(&d->m_mutex);
    d->m_caches.clear();
}

void GtDocRenderCache::renderNext()
{
    Q_D(GtDocRenderCache);

next:
    double scale = 0;
    int rotation = 0;
    int pageIndex = -1;

    // Find next task
    if (1) {
        QMutexLocker lock(&d->m_mutex);
        int i, j, c;
        GtDocRenderCachePrivate::CacheInfo *info = 0;

        c = d->m_caches.size();
        i = d->m_currentPage - d->m_index;
        i = CLAMP(i, 0, c - 1);
        for (j = i + 1; i >= 0 || j < c; --i, ++j) {
            if (i >= 0 && !d->m_caches[i].rendered) {
                info = &d->m_caches[i];
                break;
            }

            if (j < c && !d->m_caches[j].rendered) {
                info = &d->m_caches[j];
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

    // Do painting
    GtDocModel *model = d->m_view->model();
    GtDocument *document = model->document();
    if (!document) {
        qWarning() << "document is null when rendering";
        return;
    }

    GtDocPage *page = document->page(pageIndex);
    QSize size = page->size(scale, rotation);
    QImage image(size, QImage::Format_ARGB32);

    image.fill(QColor(255, 255, 255));
    page->paint(&image, scale, rotation);

    // Notify UI thread
    GtDocRenderCachePrivate::CacheInfo *info = 0;
    if (1) {
        QMutexLocker lock(&d->m_mutex);

        info = d->cacheInfo(pageIndex);
        if (info)
            info->image = image;
    }

    if (info) {
        emit finished(page->index());
    }

    goto next;
}

GT_END_NAMESPACE
