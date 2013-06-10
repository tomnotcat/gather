/*
 * Copyright (C) 2013 Tom Wong. All rights reserved.
 */
#include "pdfpage.h"
#include "pdfdocument.h"
#include <QtCore/QDebug>
#include <QtGui/QImage>

GT_BEGIN_NAMESPACE

PdfPage::PdfPage(fz_document *d, fz_page *p)
    : document(d)
    , page(p)
{
}

PdfPage::~PdfPage()
{
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
    return 0;
}

void PdfPage::extractText(ushort *texts, QRectF *rects)
{
    Q_UNUSED(texts);
    Q_UNUSED(rects);
}

void PdfPage::paint(QPaintDevice *device, double scale, int rotation)
{
    fz_context *context;
    fz_colorspace *colorspace;
    fz_pixmap *pixmap;
    fz_device *idev;
    fz_device *mdev;
    fz_display_list *list;
    QImage *image;
    fz_matrix matrix;
    fz_rect bounds;
    fz_irect ibounds;
    fz_cookie cookie = { 0, 0, 0, 0 };

#ifdef _WIN32
    colorspace = fz_device_bgr;
#else
    colorspace = fz_device_rgb;
#endif

    context = PdfDocument::context();

    if (device->devType() == QInternal::Image)
        image = static_cast<QImage*>(device);
    else
        Q_ASSERT(0);

    fz_pre_rotate(fz_scale(&matrix, scale, scale), rotation * M_PI / 180.0);

    fz_bound_page(document, page, &bounds);
    fz_round_rect(&ibounds, fz_transform_rect(&bounds, &matrix));

    ibounds.x1 = ibounds.x0 + image->width();
    ibounds.y1 = ibounds.y0 + image->height();
    fz_rect_from_irect(&bounds, &ibounds);

    pixmap = fz_new_pixmap_with_bbox_and_data(
        context, colorspace, &ibounds, image->bits());

    idev = fz_new_draw_device(context, pixmap);
    list = fz_new_display_list(context);
    mdev = fz_new_list_device(context, list);
    fz_run_page_contents(document, page, mdev, &fz_identity, &cookie);

    // FIZME: mupdf has memory leaks when render image pages
    fz_run_display_list(list, idev, &matrix, &bounds, &cookie);

    fz_free_device(mdev);
    fz_free_device(idev);
    fz_free_display_list(context, list);
    fz_drop_pixmap(context, pixmap);
}

GT_END_NAMESPACE
