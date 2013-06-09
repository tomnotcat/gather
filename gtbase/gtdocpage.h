/*
 * Copyright (C) 2013 Tom Wong. All rights reserved.
 */
#ifndef __GT_DOC_PAGE_H__
#define __GT_DOC_PAGE_H__

#include "gtcommon.h"
#include <QtCore/qobject.h>

class QPaintDevice;

GT_BEGIN_NAMESPACE

class GtDocPagePrivate;

class GT_BASE_EXPORT GtDocPage : public QObject
{
    Q_OBJECT

public:
    explicit GtDocPage(QObject *parent = 0);
    ~GtDocPage();

public:
    virtual void pageSize(double *width, double *height) = 0;
    virtual int textLength() = 0;
    virtual void extractText(QChar *texts, QRectF *rects) = 0;
    virtual void paint(QPaintDevice *device, double scale, int rotation) = 0;

public:
    int index();

protected:
    friend class GtDocument;
    GtDocPage(GtDocPagePrivate &dd, QObject *parent);
    QScopedPointer<GtDocPagePrivate> d_ptr;

private:
    Q_DISABLE_COPY(GtDocPage)
    Q_DECLARE_PRIVATE(GtDocPage)
};

GT_END_NAMESPACE

#endif  /* __GT_DOC_PAGE_H__ */
