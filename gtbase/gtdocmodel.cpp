/*
 * Copyright (C) 2013 Tom Wong. All rights reserved.
 */
#include "gtdocmodel.h"
#include "gtdocument.h"
#include <QtCore/QDebug>
#include <limits>

GT_BEGIN_NAMESPACE

class GtDocModelPrivate
{
    Q_DECLARE_PUBLIC(GtDocModel)

public:
    GtDocModelPrivate();
    ~GtDocModelPrivate();

protected:
    GtDocModel *q_ptr;
    GtDocument *document;
    int pageCount;
    int page;
    double scale;
    double maxScale;
    double minScale;
    int rotation;
    bool continuous;
    GtDocModel::LayoutMode layoutMode;
    GtDocModel::SizingMode sizingMode;
    GtDocModel::MouseMode mouseMode;
};

GtDocModelPrivate::GtDocModelPrivate()
    : document(NULL)
    , pageCount(0)
    , page(-1)
    , scale(1.)
    , maxScale(std::numeric_limits<double>::max())
    , minScale(0.)
    , rotation(0)
    , continuous(true)
    , layoutMode(GtDocModel::SinglePage)
    , sizingMode(GtDocModel::FitWidth)
    , mouseMode(GtDocModel::BrowseMode)
{
}

GtDocModelPrivate::~GtDocModelPrivate()
{
}

GtDocModel::GtDocModel(QObject *parent)
    : QObject(parent)
    , d_ptr(new GtDocModelPrivate())
{
    d_ptr->q_ptr = this;
}

GtDocModel::GtDocModel(GtDocument *document, QObject *parent)
    : QObject(parent)
    , d_ptr(new GtDocModelPrivate())
{
    d_ptr->q_ptr = this;

    if (document) {
        d_ptr->document = document;
        d_ptr->pageCount = document->pageCount();

        if (d_ptr->pageCount > 0)
            d_ptr->page = 0;

        connect(d_ptr->document,
                SIGNAL(destroyed(QObject*)),
                this,
                SLOT(documentDestroyed(QObject*)));
    }
}

GtDocModel::~GtDocModel()
{
}

GtDocument* GtDocModel::document() const
{
    Q_D(const GtDocModel);
    return d->document;
}

void GtDocModel::setDocument(GtDocument *document)
{
    Q_D(GtDocModel);

    if (document == d->document)
        return;

    if (d->document) {
        disconnect(d->document,
                   SIGNAL(destroyed(QObject*)),
                   this,
                   SLOT(documentDestroyed(QObject*)));
    }

    d->document = document;
    if (d->document) {
        d->pageCount = document->pageCount();
        setPage(CLAMP(d->page, 0, d->pageCount - 1));

        connect(d->document,
                SIGNAL(destroyed(QObject*)),
                this,
                SLOT(documentDestroyed(QObject*)));
    }

    emit documentChanged(d->document);
}

int GtDocModel::page() const
{
    Q_D(const GtDocModel);
    return d->page;
}

void GtDocModel::setPage(int page)
{
    Q_D(GtDocModel);

    if (d->page == page)
        return;

    if (page < 0 || (d->document && page >= d->pageCount))
        return;

    d->page = page;

    emit pageChanged(d->page);
}

double GtDocModel::scale() const
{
    Q_D(const GtDocModel);
    return d->scale;
}

void GtDocModel::setScale(double scale)
{
    Q_D(GtDocModel);

    double realScale = CLAMP (scale,
                              d->sizingMode == FreeSize ?
                              d->minScale : 0, d->maxScale);
    if (realScale == d->scale)
        return;

    d->scale = realScale;

    emit scaleChanged(d->scale);
}

double GtDocModel::maxScale() const
{
    Q_D(const GtDocModel);
    return d->maxScale;
}

void GtDocModel::setMaxScale(double maxScale)
{
    Q_D(GtDocModel);

    if (maxScale == d->maxScale)
        return;

    d->maxScale = maxScale;

    if (d->scale > maxScale)
        setScale(maxScale);
}

double GtDocModel::minScale() const
{
    Q_D(const GtDocModel);
    return d->minScale;
}

void GtDocModel::setMinScale(double minScale)
{
    Q_D(GtDocModel);

    if (minScale == d->minScale)
        return;

    d->minScale = minScale;

    if (d->scale < minScale)
        setScale(minScale);
}

int GtDocModel::rotation() const
{
    Q_D(const GtDocModel);
    return d->rotation;
}

void GtDocModel::setRotation(int rotation)
{
    Q_D(GtDocModel);

    int realRotation = rotation;

    if (realRotation >= 360)
        realRotation -= 360;
    else if (realRotation < 0)
        realRotation += 360;

    if (realRotation == d->rotation)
        return;

    d->rotation = realRotation;

    emit rotationChanged(d->rotation);
}

bool GtDocModel::continuous() const
{
    Q_D(const GtDocModel);
    return d->continuous;
}

void GtDocModel::setContinuous(bool continuous)
{
    Q_D(GtDocModel);

    if (continuous == d->continuous)
        return;

    d->continuous = continuous;

    emit continuousChanged(d->continuous);
}

GtDocModel::LayoutMode GtDocModel::layoutMode() const
{
    Q_D(const GtDocModel);
    return d->layoutMode;
}

void GtDocModel::setLayoutMode(LayoutMode mode)
{
    Q_D(GtDocModel);

    if (mode == d->layoutMode)
        return;

    d->layoutMode = mode;

    emit layoutModeChanged(d->layoutMode);
}

GtDocModel::SizingMode GtDocModel::sizingMode() const
{
    Q_D(const GtDocModel);
    return d->sizingMode;
}

void GtDocModel::setSizingMode(SizingMode mode)
{
    Q_D(GtDocModel);

    if (mode == d->sizingMode)
        return;

    d->sizingMode = mode;

    emit sizingModeChanged(d->sizingMode);
}

GtDocModel::MouseMode GtDocModel::mouseMode() const
{
    Q_D(const GtDocModel);
    return d->mouseMode;
}

void GtDocModel::setMouseMode(MouseMode mode)
{
    Q_D(GtDocModel);

    if (mode == d->mouseMode)
        return;

    d->mouseMode = mode;

    emit mouseModeChanged(d->mouseMode);
}

void GtDocModel::documentDestroyed(QObject *object)
{
    Q_D(GtDocModel);

    if (object == static_cast<QObject *>(d->document))
        setDocument(0);
}

GT_END_NAMESPACE
