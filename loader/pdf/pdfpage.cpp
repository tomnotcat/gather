/*
 * Copyright (C) 2013 Tom Wong. All rights reserved.
 */
#include "pdfpage.h"
#include "pdfdocument.h"
#include <QtCore/QDebug>
#include <QtGui/QImage>

GT_BEGIN_NAMESPACE

PdfPage::PdfPage(fz_context *c, fz_document *d, fz_page *p)
    : context(c)
    , document(d)
    , page(p)
    , pageList(0)
    , pageText(0)
    , pageSheet(0)
{
}

PdfPage::~PdfPage()
{
    if (pageList)
        fz_free_display_list(context, pageList);

    if (pageText)
        fz_free_text_page(context, pageText);

    if (pageSheet)
        fz_free_text_sheet(context, pageSheet);

    fz_free_page(document, page);
}

void PdfPage::size(double *width, double *height)
{
    fz_rect bbox;
    fz_bound_page(document, page, &bbox);
    *width = bbox.x1;
    *height = bbox.y1;
}

int PdfPage::textLength()
{
    loadContent();

    fz_text_page *page = pageText;
    fz_text_block *block;
    fz_text_line *line;
    fz_text_span *span;
    int len = 0;

    for (block = page->blocks; block < page->blocks + page->len; block++) {
        for (line = block->lines; line < block->lines + block->len; line++) {
            for (span = line->spans; span < line->spans + line->len; span++)
                len += span->len;

            /* pseudo-newline */
            len++;
        }
    }

    return len;
}

int PdfPage::extractText(QChar *texts, QRectF *rects, int length)
{
    loadContent();

    fz_text_page *page = pageText;
    fz_text_block *block;
    fz_text_line *line;
    fz_text_span *span;
    fz_rect bbox;
    int c, i, p = 0;

    for (block = page->blocks; block < page->blocks + page->len; block++) {
        for (line = block->lines; line < block->lines + block->len; line++) {
            for (span = line->spans; span < line->spans + line->len; span++) {
                for (i = 0; i < span->len; i++) {
                    c = span->text[i].c;
                    if (c < 32)
                        c = '?';

                    bbox = span->text[i].bbox;
                    rects[p].setCoords(bbox.x0, bbox.y0, bbox.x1, bbox.y1);
                    texts[p++] = c;
                }
            }

            if (p > 0) {
                int x = rects[p - 1].x() + rects[p - 1].width();
                int y = rects[p - 1].y();
                rects[p].setCoords(x, y, x + 1, y + rects[p - 1].height());
            }
            else {
                rects[p].setCoords(0, 0, 0, 0);
            }

            texts[p++] = '\n';
        }
    }

    Q_ASSERT(p == length);
    return p;
}

void PdfPage::paint(QPaintDevice *device, double scale, int rotation)
{
    fz_colorspace *colorspace;
    fz_pixmap *pixmap;
    fz_device *idev;
    QImage *image;
    fz_matrix matrix;
    fz_rect bounds;
    fz_irect ibounds;
    fz_cookie cookie = { 0, 0, 0, 0 };

    loadContent();

#ifdef _WIN32
    colorspace = fz_device_bgr;
#else
    colorspace = fz_device_rgb;
#endif

    if (device->devType() == QInternal::Image)
        image = static_cast<QImage*>(device);
    else
        Q_ASSERT(0);

    fz_pre_rotate(fz_scale(&matrix, scale, scale), rotation);

    fz_bound_page(document, page, &bounds);
    fz_round_rect(&ibounds, fz_transform_rect(&bounds, &matrix));

    ibounds.x1 = ibounds.x0 + image->width();
    ibounds.y1 = ibounds.y0 + image->height();
    fz_rect_from_irect(&bounds, &ibounds);

    pixmap = fz_new_pixmap_with_bbox_and_data(
        context, colorspace, &ibounds, image->bits());

    idev = fz_new_draw_device(context, pixmap);

    // FIZME: mupdf has memory leaks when render image pages
    fz_run_display_list(pageList, idev, &matrix, &bounds, &cookie);

    fz_free_device(idev);
    fz_drop_pixmap(context, pixmap);
}

void PdfPage::loadContent()
{
    fz_device *mdev;
    fz_device *tdev;
    fz_rect bbox;
    fz_cookie cookie = { 0, 0, 0, 0 };

    if (!pageList) {
        pageList = fz_new_display_list(context);
        mdev = fz_new_list_device(context, pageList);
        fz_run_page_contents(document, page, mdev, &fz_identity, &cookie);
        fz_free_device(mdev);
    }

    if (!pageText) {
        fz_bound_page(document, page, &bbox);
        pageSheet = fz_new_text_sheet(context);
        pageText = fz_new_text_page(context, &bbox);

        tdev = fz_new_text_device(context, pageSheet, pageText);
        if (pageList) {
            fz_run_display_list(pageList,
                                tdev,
                                &fz_identity,
                                &fz_infinite_rect,
                                &cookie);
        }

        fz_free_device(tdev);
    }
}

GT_END_NAMESPACE
