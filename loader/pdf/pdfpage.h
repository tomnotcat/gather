/*
 * Copyright (C) 2013 Tom Wong. All rights reserved.
 */
#ifndef __PDF_PAGE_H__
#define __PDF_PAGE_H__

#include "gtabstractdocument.h"
#include <QtCore/QString>

extern "C" {
#include "mupdf-internal.h"
}

GT_BEGIN_NAMESPACE

class PdfPage : public GtAbstractPage
{
public:
    PdfPage(fz_context *c, fz_document *d, fz_page *p, const QString &l);
    ~PdfPage();

public:
    QString label();
    void size(double *width, double *height);
    int textLength();
    int extractText(QChar *texts, QRectF *rects, int length);
    void paint(QPaintDevice *device, double scale, int rotation);

protected:
    void loadContent();
    inline int texthash(const fz_text_char &c) {
        return (c.c << 16) | ((int)c.bbox.x0);
    }

private:
    fz_context *context;
    fz_document *document;
    fz_page *page;
    fz_display_list *pageList;
    fz_display_list *annotationList;
    fz_text_page *pageText;
    fz_text_sheet *pageSheet;
    QString _label;
};

GT_END_NAMESPACE

#endif  /* __PDF_PAGE_H__ */
