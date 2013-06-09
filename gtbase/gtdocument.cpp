/*
 * Copyright (C) 2013 Tom Wong. All rights reserved.
 */
#include "gtdocument_p.h"
#include "gtdocpage_p.h"
#include <QtCore/QDebug>

GT_BEGIN_NAMESPACE

GtDocumentPrivate::GtDocumentPrivate()
    : device(0)
    , pages(0)
    , pageCount(0)
    , uniformWidth(0)
    , uniformHeight(0)
    , maxWidth(0)
    , maxHeight(0)
    , minWidth(0)
    , minHeight(0)
    , uniform(false)
    , loaded(false)
    , destroyed(false)
{
}

GtDocumentPrivate::~GtDocumentPrivate()
{
    Q_ASSERT(destroyed);
}

void GtDocumentPrivate::setDevice(QIODevice *device)
{
    Q_ASSERT(0 == this->device);

    this->device = device;

    q_ptr->connect(device,
                   SIGNAL(destroyed(QObject*)),
                   q_ptr,
                   SLOT(deviceDestroyed(QObject*)));
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

QIODevice* GtDocument::device()
{
    Q_D(GtDocument);
    return d->device;
}

bool GtDocument::isLoaded()
{
    Q_D(GtDocument);
    return d->loaded;
}

bool GtDocument::uniformPageSize(double *width, double *height)
{
    Q_D(GtDocument);

    Q_ASSERT(d->loaded);

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

    Q_ASSERT(d->loaded);

    if (width)
        *width = d->maxWidth;

    if (height)
        *height = d->maxHeight;
}

void GtDocument::minPageSize(double *width, double *height)
{
    Q_D(GtDocument);

    Q_ASSERT(d->loaded);

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

GtDocPage* GtDocument::page(int index)
{
    Q_D(GtDocument);

    if (index < 0 || index >= d->pageCount)
        return 0;

    return d->pages[index];
}

QSize GtDocument::pageSizeForScaleRotation(int index, double scale, int rotation)
{
    double width, height;

    page(index)->pageSize(&width, &height);

    width *= scale;
    height *= scale;

    if (rotation == 0 || rotation == 180)
        return QSize(width + 0.5, height + 0.5);

    Q_ASSERT(rotation == 90 || rotation == 270);
    return QSize(height + 0.5, width + 0.5);
}

void GtDocument::deviceDestroyed(QObject*)
{
    Q_D(GtDocument);

    // The device should not release before document destroy
    Q_ASSERT(d->destroyed);
}

void GtDocument::slotLoadDocument()
{
    Q_D(GtDocument);

    Q_ASSERT(!d->loaded);

    if (!loadDocument()) {
        emit loaded(this);
        return;
    }

    d->pageCount = countPages();
    if (d->pageCount > 0) {
        double pageWidth, pageHeight;

        d->pages = new GtDocPage*[d->pageCount];
        d->uniform = true;
        for (int i = 0; i < d->pageCount; ++i) {
            d->pages[i] = loadPage(i);
            if (0 == d->pages[i]) {
                qWarning() << "load page failed:" << i;
                continue;
            }

            d->pages[i]->d_ptr->index = i;
            d->pages[i]->pageSize(&pageWidth, &pageHeight);
            if (i == 0) {
                d->uniformWidth = pageWidth;
                d->uniformHeight = pageHeight;
                d->maxWidth = d->uniformWidth;
                d->maxHeight = d->uniformHeight;
                d->minWidth = d->uniformWidth;
                d->minHeight = d->uniformHeight;
            }
            else if (d->uniform &&
                     (d->uniformWidth != pageWidth ||
                      d->uniformHeight != pageHeight))
            {
                d->uniform = false;
            }

            if (!d->uniform) {
                if (pageWidth > d->maxWidth)
                    d->maxWidth = pageWidth;

                if (pageWidth < d->minWidth)
                    d->minWidth = pageWidth;

                if (pageHeight > d->maxHeight)
                    d->maxHeight = pageHeight;

                if (pageHeight < d->minHeight)
                    d->minHeight = pageHeight;
            }
        }
    }

    d->loaded = true;
    emit loaded(this);
}

void GtDocument::destroy()
{
    Q_D(GtDocument);

    Q_ASSERT(!d->destroyed);

    for (int i = 0; i < d->pageCount; ++i)
        delete d->pages[i];

    delete[] d->pages;

    d->destroyed = true;
}

GT_END_NAMESPACE
