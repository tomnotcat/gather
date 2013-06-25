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

GtDocText::GtDocText(const GtDocText &o)
    : QSharedData(o)
{
    // this should not happen
    Q_ASSERT(0);
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

QString GtDocPage::label()
{
    Q_D(GtDocPage);

    if (d->label.isNull()) {
        GtAbstractPage *abstractPage = d->document->d_ptr->lockPage(d->index);

        d->label = abstractPage->label();
        if (d->label.isNull())
            d->label = QString("");

        d->document->d_ptr->unlockPage(d->index);
    }

    return d->label;
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

int GtDocPage::length()
{
    Q_D(GtDocPage);

    if (-1 == d->textLength) {
        GtAbstractPage *abstractPage = d->document->d_ptr->lockPage(d->index);
        d->textLength = abstractPage->textLength();
        d->document->d_ptr->unlockPage(d->index);
    }

    return d->textLength;
}

const QSharedDataPointer<GtDocText> GtDocPage::text()
{
    Q_D(GtDocPage);

    QSharedDataPointer<GtDocText> r(d->text);
    if (!r) {
        QChar *texts = 0;
        QRectF *rects = 0;
        GtAbstractPage *abstractPage = d->document->d_ptr->lockPage(d->index);

        if (-1 == d->textLength)
            d->textLength = abstractPage->textLength();

        if (d->textLength > 0) {
            texts = new QChar[d->textLength];
            rects = new QRectF[d->textLength];
            d->abstractPage->extractText(texts, rects, d->textLength);
        }

        d->document->d_ptr->unlockPage(d->index);
        r = new GtDocText(texts, rects, d->textLength);
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
