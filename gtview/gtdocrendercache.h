/*
 * Copyright (C) 2013 Tom Wong. All rights reserved.
 */
#ifndef __GT_DOC_RENDER_CACHE_H__
#define __GT_DOC_RENDER_CACHE_H__

#include "gtcommon.h"
#include <QtCore/QObject>

GT_BEGIN_NAMESPACE

class GtDocModel;
class GtDocRenderCachePrivate;

class GtDocRenderCache : public QObject
{
    Q_OBJECT

public:
    explicit GtDocRenderCache(QObject *parent = 0);
    ~GtDocRenderCache();

public:
    void setModel(GtDocModel *model);
    void setMaxSize(int maxSize);
    void setPageRange(int beginPage, int endPage);
    QImage* pageImage(int index);
    void clear(void);

private:
    QScopedPointer<GtDocRenderCachePrivate> d_ptr;

private:
    Q_DISABLE_COPY(GtDocRenderCache)
    Q_DECLARE_PRIVATE(GtDocRenderCache)
};

GT_END_NAMESPACE

#endif  /* __GT_DOC_RENDER_CACHE_H__ */
