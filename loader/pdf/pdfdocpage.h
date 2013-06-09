/*
 * Copyright (C) 2013 Tom Wong. All rights reserved.
 */
#ifndef __PDF_DOC_PAGE_H__
#define __PDF_DOC_PAGE_H__

#include "gtdocpage.h"

extern "C" {
#include "mupdf-internal.h"
}

GT_BEGIN_NAMESPACE

class PdfDocPagePrivate;

class PdfDocPage : public GtDocPage
{
    Q_OBJECT

public:
    PdfDocPage(fz_document *document, fz_page *page, QObject *parent = 0);
    ~PdfDocPage();

public:
    void pageSize(double *width, double *height);
    int textLength();
    void extractText(QChar *texts, QRectF *rects);
    void paint(QPaintDevice *device, double scale, int rotation);

private:
    Q_DISABLE_COPY(PdfDocPage)
    Q_DECLARE_PRIVATE(PdfDocPage)
};

GT_END_NAMESPACE

#endif  /* __PDF_DOC_PAGE_H__ */
