/*
 * Copyright (C) 2013 Tom Wong. All rights reserved.
 */
#include "gtdocument_p.h"
#include "gtdocpage.h"
#include <QtCore/QDebug>

GT_BEGIN_NAMESPACE

GtDocumentPrivate::GtDocumentPrivate()
    : pages(0)
    , pageCount(0)
    , uniformWidth(0)
    , uniformHeight(0)
    , maxWidth(0)
    , maxHeight(0)
    , minWidth(0)
    , minHeight(0)
    , uniform(false)
    , initialized(false)
{
}

GtDocumentPrivate::~GtDocumentPrivate()
{
    for (int i = 0; i < pageCount; ++i)
        delete pages[i];

    delete[] pages;
}

void GtDocumentPrivate::initialize()
{
    Q_Q(GtDocument);

    Q_ASSERT(!initialized);

    pageCount = q->countPages();
    if (pageCount > 0) {
        double pageWidth, pageHeight;

        pages = new GtDocPage*[pageCount];
        uniform = true;
        for (int i = 0; i < pageCount; ++i) {
            pages[i] = q->loadPage(i);
            if (0 == pages[i]) {
                qWarning() << "load page failed:" << i;
                continue;
            }

            pages[i]->getSize(&pageWidth, &pageHeight);
            if (i == 0) {
                uniformWidth = pageWidth;
                uniformHeight = pageHeight;
                maxWidth = uniformWidth;
                maxHeight = uniformHeight;
                minWidth = uniformWidth;
                minHeight = uniformHeight;
            }
            else if (uniform &&
                     (uniformWidth != pageWidth ||
                      uniformHeight != pageHeight))
            {
                uniform = false;
            }

            if (!uniform) {
                if (pageWidth > maxWidth)
                    maxWidth = pageWidth;

                if (pageWidth < minWidth)
                    minWidth = pageWidth;

                if (pageHeight > maxHeight)
                    maxHeight = pageHeight;

                if (pageHeight < minHeight)
                    minHeight = pageHeight;
            }
        }
    }

    initialized = true;
}

GtDocument::GtDocument(QObject *parent)
    : QObject(parent)
    , d_ptr(new GtDocumentPrivate())
{
    d_ptr->q_ptr = this;
}

GtDocument::GtDocument(GtDocumentPrivate &dd, QObject *parent)
    : QObject(parent)
    , d_ptr(&dd)
{
    d_ptr->q_ptr = this;
}

GtDocument::~GtDocument()
{
}

bool GtDocument::uniformPageSize(double *width, double *height)
{
    Q_D(GtDocument);

    Q_ASSERT(d->initialized);

    if (d->uniform) {
        if (width)
            *width = d->uniformWidth;

        if (height)
            *height = d->uniformHeight;
    }

    return d->uniform;
}

void GtDocument::maxPageSize(double *width, double *height)
{
    Q_D(GtDocument);

    Q_ASSERT(d->initialized);

    if (width)
        *width = d->maxWidth;

    if (height)
        *height = d->maxHeight;
}

void GtDocument::minPageSize(double *width, double *height)
{
    Q_D(GtDocument);

    Q_ASSERT(d->initialized);

    if (width)
        *width = d->minWidth;

    if (height)
        *height = d->minHeight;
}

int GtDocument::pageCount()
{
    Q_D(GtDocument);
    return d->pageCount;
}

GtDocPage* GtDocument::page(int page)
{
    Q_D(GtDocument);

    if (page < 0 || page >= d->pageCount)
        return 0;

    return d->pages[page];
}

GT_END_NAMESPACE
