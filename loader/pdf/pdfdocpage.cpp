/*
 * Copyright (C) 2013 Tom Wong. All rights reserved.
 */
#include "pdfdocpage.h"
#include "gtdocpage_p.h"

GT_BEGIN_NAMESPACE

class PdfDocPagePrivate : public GtDocPagePrivate
{
    Q_DECLARE_PUBLIC(GtDocPage)

public:
    PdfDocPagePrivate();
    ~PdfDocPagePrivate();
};

PdfDocPagePrivate::PdfDocPagePrivate()
{
}

PdfDocPagePrivate::~PdfDocPagePrivate()
{
}

PdfDocPage::PdfDocPage(QObject *parent)
    : GtDocPage(*new GtDocPagePrivate(), parent)
{
}

PdfDocPage::~PdfDocPage()
{
}

void PdfDocPage::getSize(double *width, double *height)
{
    Q_UNUSED(width);
    Q_UNUSED(height);
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
