/*
 * Copyright (C) 2013 Tom Wong. All rights reserved.
 */
#include "pdfoutline.h"
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

QString PdfOutline::title(void *it)
{
    fz_outline *l = static_cast<fz_outline*>(it);
    return QString::fromUtf8(l->title);
}

int PdfOutline::page(void *it)
{
    fz_outline *l = static_cast<fz_outline*>(it);
    return l->dest.kind == FZ_LINK_GOTO ? l->dest.ld.gotor.page : -1;
}

GT_END_NAMESPACE
