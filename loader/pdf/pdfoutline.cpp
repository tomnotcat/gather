/*
 * Copyright (C) 2013 Tom Wong. All rights reserved.
 */
#include "pdfoutline.h"
#include "gtlinkdest.h"
#include <QtCore/QDebug>

GT_BEGIN_NAMESPACE

PdfOutline::PdfOutline(fz_context *c, fz_outline *o)
    : context(c)
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

    if (l->dest.kind == FZ_LINK_GOTO)
        return GtLinkDest(l->dest.ld.gotor.page);

    return GtLinkDest();
}

GT_END_NAMESPACE
