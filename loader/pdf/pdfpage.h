/*
 * Copyright (C) 2013 Tom Wong. All rights reserved.
 */
#ifndef __PDF_PAGE_H__
#define __PDF_PAGE_H__

#include "gtabstractdocument.h"

extern "C" {
#include "mupdf-internal.h"
}

GT_BEGIN_NAMESPACE

class PdfPage : public GtAbstractPage
{
public:
    PdfPage(fz_context *c, fz_document *d, fz_page *p);
    ~PdfPage();

public:
    void size(double *width, double *height);
    int textLength();
    void extractText(QChar *texts, QRectF *rects);
    void paint(QPaintDevice *device, double scale, int rotation);

protected:
    void loadContent();

private:
    fz_context *context;
    fz_document *document;
    fz_page *page;
    fz_display_list *pageList;
    fz_text_page *pageText;
    fz_text_sheet *pageSheet;
};

GT_END_NAMESPACE

#endif  /* __PDF_PAGE_H__ */
