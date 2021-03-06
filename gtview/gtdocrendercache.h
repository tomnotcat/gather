/*
 * Copyright (C) 2013 Tom Wong. All rights reserved.
 */
#ifndef __GT_DOC_RENDER_CACHE_H__
#define __GT_DOC_RENDER_CACHE_H__

#include "gtobject.h"
#include <QtCore/QObject>

GT_BEGIN_NAMESPACE

class GtDocView;
class GtDocRenderCachePrivate;

class GtDocRenderCache : public QObject, public GtObject
{
    Q_OBJECT

public:
    explicit GtDocRenderCache(GtDocView *view, QObject *parent = 0);
    ~GtDocRenderCache();

public:
    void setMaxSize(int maxSize);
    void setPageRange(int beginPage, int endPage, int currentPage);
    QImage image(int index);
    void clear();

Q_SIGNALS:
    void finished(int index);

private Q_SLOTS:
    void renderNext();

private:
    QScopedPointer<GtDocRenderCachePrivate> d_ptr;

private:
    Q_DISABLE_COPY(GtDocRenderCache)
    Q_DECLARE_PRIVATE(GtDocRenderCache)
};

GT_END_NAMESPACE

#endif  /* __GT_DOC_RENDER_CACHE_H__ */
