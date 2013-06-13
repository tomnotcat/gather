/*
 * Copyright (C) 2013 Tom Wong. All rights reserved.
 */
#include "gtdocpage_p.h"
#include "gtabstractdocument.h"
#include "gtdocument_p.h"
#include <QtCore/QDebug>
#include <QtCore/QMutex>
#include <QtCore/QRectF>

GT_BEGIN_NAMESPACE

GtDocText::GtDocText(QChar *texts, QRectF *rects, int length)
    : _texts(texts)
    , _rects(rects)
    , _length(length)
{
}

GtDocText::~GtDocText()
{
    delete[] _texts;
    delete[] _rects;
}

GtDocPagePrivate::GtDocPagePrivate()
    : abstractPage(0)
    , document(0)
    , index(-1)
    , textLength(-1)
    , width(0)
    , height(0)
{
}

GtDocPagePrivate::~GtDocPagePrivate()
{
    delete abstractPage;
}

GtDocPage::GtDocPage(QObject *parent)
    : QObject(parent)
    , d_ptr(new GtDocPagePrivate())
{
    d_ptr->q_ptr = this;
}

GtDocPage::GtDocPage(GtDocPagePrivate &dd, QObject *parent)
    : QObject(parent)
    , d_ptr(&dd)
{
    d_ptr->q_ptr = this;
}

GtDocPage::~GtDocPage()
{
}

GtDocument* GtDocPage::document()
{
    Q_D(GtDocPage);
    return d->document;
}

int GtDocPage::index()
{
    Q_D(GtDocPage);
    return d->index;
}

void GtDocPage::size(double *width, double *height)
{
    Q_D(GtDocPage);

    if (width)
        *width = d->width;

    if (height)
        *height = d->height;
}

QSize GtDocPage::size(double scale, int rotation)
{
    Q_D(GtDocPage);

    double width = d->width * scale;
    double height = d->height * scale;

    if (rotation == 0 || rotation == 180)
        return QSize(width + 0.5, height + 0.5);

    Q_ASSERT(rotation == 90 || rotation == 270);
    return QSize(height + 0.5, width + 0.5);
}

QSharedDataPointer<GtDocText> GtDocPage::text()
{
    Q_D(GtDocPage);

    QSharedDataPointer<GtDocText> r(d->text);
    if (!r) {
        QChar *texts = 0;
        QRectF *rects = 0;
        GtAbstractPage *abstractPage = d->document->d_ptr->lockPage(d->index);
        int length = abstractPage->textLength();

        if (length > 0) {
            texts = new QChar[length];
            rects = new QRectF[length];
            d->abstractPage->extractText(texts, rects);
        }

        d->document->d_ptr->unlockPage(d->index);
        r = new GtDocText(texts, rects, length);
        d->document->d_ptr->cacheText(d->index, r);
    }

    return r;
}

void GtDocPage::paint(QPaintDevice *device, double scale, int rotation)
{
    Q_D(GtDocPage);

    GtAbstractPage *abstractPage = d->document->d_ptr->lockPage(d->index);
    abstractPage->paint(device, scale, rotation);
    d->document->d_ptr->unlockPage(d->index);
}

GT_END_NAMESPACE
