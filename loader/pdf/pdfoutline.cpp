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

void* PdfOutline::iterator()
{
    return outline;
}

void* PdfOutline::nextIterator(void *it)
{
    fz_outline *l = static_cast<fz_outline*>(it);
    return l->next;
}

void* PdfOutline::childIterator(void *it)
{
    fz_outline *l = static_cast<fz_outline*>(it);
    return l->down;
}

void* PdfOutline::freeIterator(void *)
{
    return 0;
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
