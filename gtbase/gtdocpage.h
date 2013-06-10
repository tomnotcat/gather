/*
 * Copyright (C) 2013 Tom Wong. All rights reserved.
 */
#ifndef __GT_DOC_PAGE_H__
#define __GT_DOC_PAGE_H__

#include "gtcommon.h"
#include <QtCore/QObject>
#include <QtCore/QSize>

class QPaintDevice;

GT_BEGIN_NAMESPACE

class GtDocPagePrivate;
class GtDocument;

class GT_BASE_EXPORT GtDocPage : public QObject
{
    Q_OBJECT

public:
    explicit GtDocPage(QObject *parent = 0);
    ~GtDocPage();

public:
    GtDocument* document();
    int index();
    void size(double *width, double *height);
    QSize size(double scale = 1.0, int rotation = 0);
    int textLength();
    void extractText(ushort *texts, QRectF *rects);
    void paint(QPaintDevice *device, double scale = 1.0, int rotation = 0);

protected:
    friend class GtDocument;
    friend class GtDocumentPrivate;
    GtDocPage(GtDocPagePrivate &dd, QObject *parent);
    QScopedPointer<GtDocPagePrivate> d_ptr;

private:
    Q_DISABLE_COPY(GtDocPage)
    Q_DECLARE_PRIVATE(GtDocPage)
};

GT_END_NAMESPACE

#endif  /* __GT_DOC_PAGE_H__ */
