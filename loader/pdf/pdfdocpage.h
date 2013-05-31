/*
 * Copyright (C) 2013 Tom Wong. All rights reserved.
 */
#ifndef __PDF_DOC_PAGE_H__
#define __PDF_DOC_PAGE_H__

#include "gtdocpage.h"

GT_BEGIN_NAMESPACE

class PdfDocPagePrivate;

class PdfDocPage : public GtDocPage
{
    Q_OBJECT

public:
    explicit PdfDocPage(QObject *parent = 0);
    ~PdfDocPage();

public:
    void getSize(double *width, double *height);
    int textLength();
    void extractText(QChar *texts, QRectF *rects);
    void paint(QPainter &p, double scale, int rotation);

private:
    Q_DISABLE_COPY(PdfDocPage)
    Q_DECLARE_PRIVATE(PdfDocPage)
};

GT_END_NAMESPACE

#endif  /* __PDF_DOC_PAGE_H__ */
