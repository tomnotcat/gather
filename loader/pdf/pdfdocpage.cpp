/*
 * Copyright (C) 2013 Tom Wong. All rights reserved.
 */
#include "pdfdocpage.h"
#include "gtdocpage_p.h"
#include <QtCore/QDebug>

GT_BEGIN_NAMESPACE

class PdfDocPagePrivate : public GtDocPagePrivate
{
    Q_DECLARE_PUBLIC(PdfDocPage)

public:
    PdfDocPagePrivate(fz_document *d, fz_page *p);
    ~PdfDocPagePrivate();

private:
    fz_document *document;
    fz_page *page;
    fz_rect bbox;
};

PdfDocPagePrivate::PdfDocPagePrivate(fz_document *d, fz_page *p)
    : document(d)
    , page(p)
{
}

PdfDocPagePrivate::~PdfDocPagePrivate()
{
    fz_free_page(document, page);
}

PdfDocPage::PdfDocPage(fz_document *document, fz_page *page, QObject *parent)
    : GtDocPage(*new PdfDocPagePrivate(document, page), parent)
{
}

PdfDocPage::~PdfDocPage()
{
}

void PdfDocPage::getSize(double *width, double *height)
{
    Q_D(PdfDocPage);

    if (width)
        *width = d->bbox.x1;

    if (height)
        *height = d->bbox.y1;
}

int PdfDocPage::textLength()
{
    return 0;
}

void PdfDocPage::extractText(QChar *texts, QRectF *rects)
{
    Q_UNUSED(texts);
    Q_UNUSED(rects);
}

void PdfDocPage::paint(QPainter &p, double scale, int rotation)
{
    Q_UNUSED(p);
    Q_UNUSED(scale);
    Q_UNUSED(rotation);
}

GT_END_NAMESPACE
