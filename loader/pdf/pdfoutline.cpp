/*
 * Copyright (C) 2013 Tom Wong. All rights reserved.
 */
#include "pdfoutline.h"
#include "gtlinkdest.h"
#include <QtCore/QDebug>

GT_BEGIN_NAMESPACE

PdfOutline::PdfOutline(fz_context *c, fz_document *d, fz_outline *o)
    : context(c)
    , document(d)
    , outline(o)
{
}

PdfOutline::~PdfOutline()
{
    fz_free_outline(context, outline);
}

void* PdfOutline::firstNode()
{
    return outline;
}

void* PdfOutline::nextNode(void *node)
{
    fz_outline *l = static_cast<fz_outline*>(node);
    return l->next;
}

void* PdfOutline::childNode(void *node)
{
    fz_outline *l = static_cast<fz_outline*>(node);
    return l->down;
}

QString PdfOutline::title(void *node)
{
    fz_outline *l = static_cast<fz_outline*>(node);
    return QString::fromUtf8(l->title);
}

GtLinkDest PdfOutline::dest(void *node)
{
    fz_outline *l = static_cast<fz_outline*>(node);

    if (l->dest.kind == FZ_LINK_GOTO ||
        l->dest.kind == FZ_LINK_GOTOR)
    {
        double zoom = 0;
        double x = 0;
        double y = 0;

        fz_point lt = l->dest.ld.gotor.lt;
        fz_point rb = l->dest.ld.gotor.rb;

        fz_page *page = fz_load_page(document, l->dest.ld.gotor.page);
        fz_transform_point(&lt, &((pdf_page*)page)->ctm);
        fz_transform_point(&rb, &((pdf_page*)page)->ctm);
        fz_free_page(document, page);

        if ((l->dest.ld.gotor.flags & fz_link_flag_r_is_zoom)) {
            if ((l->dest.ld.gotor.flags & fz_link_flag_l_valid))
                x = lt.x;

            if ((l->dest.ld.gotor.flags & fz_link_flag_t_valid))
                y = lt.y;

            zoom = l->dest.ld.gotor.rb.x;
        }

        QPointF point(x, y);
        return GtLinkDest(l->dest.ld.gotor.page, point, zoom);
    }

    return GtLinkDest();
}

GT_END_NAMESPACE
