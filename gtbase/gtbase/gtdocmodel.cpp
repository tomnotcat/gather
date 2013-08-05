/*
 * Copyright (C) 2013 Tom Wong. All rights reserved.
 */
#include "gtdocmodel.h"
#include "gtbookmark.h"
#include "gtbookmarks.h"
#include "gtdocmeta.h"
#include "gtdocnotes.h"
#include "gtdocument.h"
#include <QtCore/QDebug>
#include <limits>

GT_BEGIN_NAMESPACE

class GtDocModelPrivate
{
    Q_DECLARE_PUBLIC(GtDocModel)

public:
    GtDocModelPrivate(GtDocModel *q);
    ~GtDocModelPrivate();

protected:
    GtDocModel *q_ptr;
    GtDocMeta *m_meta;
    GtDocument *m_document;
    GtBookmarks *m_bookmarks;
    GtDocNotes *m_notes;
    int m_pageCount;
    int m_page;
    double m_scale;
    double m_maxScale;
    double m_minScale;
    int m_rotation;
    bool m_continuous;
    GtDocModel::LayoutMode m_layoutMode;
    GtDocModel::SizingMode m_sizingMode;
    GtDocModel::MouseMode m_mouseMode;
};

GtDocModelPrivate::GtDocModelPrivate(GtDocModel *q)
    : q_ptr(q)
    , m_meta(0)
    , m_document(0)
    , m_bookmarks(0)
    , m_notes(0)
    , m_pageCount(-1)
    , m_page(-1)
    , m_scale(1.)
    , m_maxScale(std::numeric_limits<double>::max())
    , m_minScale(0.)
    , m_rotation(0)
    , m_continuous(true)
    , m_layoutMode(GtDocModel::SinglePage)
    , m_sizingMode(GtDocModel::FitWidth)
    , m_mouseMode(GtDocModel::BrowseMode)
{
}

GtDocModelPrivate::~GtDocModelPrivate()
{
    if (m_meta)
        m_meta->release();

    if (m_document)
        m_document->release();

    if (m_bookmarks)
        m_bookmarks->release();

    if (m_notes)
        m_notes->release();
}

GtDocModel::GtDocModel(QObject *parent)
    : QObject(parent)
    , d_ptr(new GtDocModelPrivate(this))
{
}

GtDocModel::~GtDocModel()
{
}

GtDocMeta* GtDocModel::meta() const
{
    Q_D(const GtDocModel);
    return d->m_meta;
}

void GtDocModel::setMeta(GtDocMeta *meta)
{
    Q_D(GtDocModel);

    if (meta == d->m_meta)
        return;

    if (d->m_meta)
        d->m_meta->release();

    d->m_meta = meta;

    if (d->m_meta)
        d->m_meta->ref.ref();

    emit metaChanged(d->m_meta);
}

GtDocument* GtDocModel::document() const
{
    Q_D(const GtDocModel);
    return d->m_document;
}

void GtDocModel::setDocument(GtDocument *document)
{
    Q_D(GtDocModel);

    if (document == d->m_document)
        return;

    if (d->m_document)
        d->m_document->release();

    d->m_document = document;
    d->m_pageCount = -1;

    if (d->m_document) {
        d->m_document->ref.ref();

        if (document->isLoaded()) {
            d->m_pageCount = document->pageCount();
            setPage(CLAMP(d->m_page, 0, d->m_pageCount - 1));
        }
    }

    if (-1 == d->m_pageCount)
        setPage(-1);

    emit documentChanged(d->m_document);
}

GtBookmarks* GtDocModel::bookmarks() const
{
    Q_D(const GtDocModel);
    return d->m_bookmarks;
}

void GtDocModel::setBookmarks(GtBookmarks *bookmarks)
{
    Q_D(GtDocModel);

    if (bookmarks == d->m_bookmarks)
        return;

    if (d->m_bookmarks)
        d->m_bookmarks->release();

    d->m_bookmarks = bookmarks;

    if (d->m_bookmarks)
        d->m_bookmarks->ref.ref();

    emit bookmarksChanged(d->m_bookmarks);
}

GtDocNotes* GtDocModel::notes() const
{
    Q_D(const GtDocModel);
    return d->m_notes;
}

void GtDocModel::setNotes(GtDocNotes *notes)
{
    Q_D(GtDocModel);

    if (notes == d->m_notes)
        return;

    if (d->m_notes)
        d->m_notes->release();

    d->m_notes = notes;

    if (d->m_notes)
        d->m_notes->ref.ref();

    emit notesChanged(d->m_notes);
}

int GtDocModel::page() const
{
    Q_D(const GtDocModel);
    return d->m_page;
}

void GtDocModel::setPage(int page)
{
    Q_D(GtDocModel);

    if (!d->m_document || d->m_page == page)
        return;

    if (-1 == d->m_pageCount && d->m_document->isLoaded())
        d->m_pageCount = d->m_document->pageCount();

    if (page < 0 || page >= d->m_pageCount)
        return;

    d->m_page = page;

    emit pageChanged(d->m_page);
}

double GtDocModel::scale() const
{
    Q_D(const GtDocModel);
    return d->m_scale;
}

void GtDocModel::setScale(double scale)
{
    Q_D(GtDocModel);

    double realScale = CLAMP(scale,
                             d->m_sizingMode == FreeSize ?
                             d->m_minScale : 0, d->m_maxScale);
    if (realScale == d->m_scale)
        return;

    d->m_scale = realScale;

    emit scaleChanged(d->m_scale);
}

double GtDocModel::maxScale() const
{
    Q_D(const GtDocModel);
    return d->m_maxScale;
}

void GtDocModel::setMaxScale(double maxScale)
{
    Q_D(GtDocModel);

    if (maxScale == d->m_maxScale)
        return;

    d->m_maxScale = maxScale;

    if (d->m_scale > maxScale)
        setScale(maxScale);
}

double GtDocModel::minScale() const
{
    Q_D(const GtDocModel);
    return d->m_minScale;
}

void GtDocModel::setMinScale(double minScale)
{
    Q_D(GtDocModel);

    if (minScale == d->m_minScale)
        return;

    d->m_minScale = minScale;

    if (d->m_scale < minScale)
        setScale(minScale);
}

int GtDocModel::rotation() const
{
    Q_D(const GtDocModel);
    return d->m_rotation;
}

void GtDocModel::setRotation(int rotation)
{
    Q_D(GtDocModel);

    int realRotation = rotation;

    if (realRotation >= 360)
        realRotation -= 360;
    else if (realRotation < 0)
        realRotation += 360;

    if (realRotation == d->m_rotation)
        return;

    d->m_rotation = realRotation;

    emit rotationChanged(d->m_rotation);
}

bool GtDocModel::continuous() const
{
    Q_D(const GtDocModel);
    return d->m_continuous;
}

void GtDocModel::setContinuous(bool continuous)
{
    Q_D(GtDocModel);

    if (continuous == d->m_continuous)
        return;

    d->m_continuous = continuous;

    emit continuousChanged(d->m_continuous);
}

GtDocModel::LayoutMode GtDocModel::layoutMode() const
{
    Q_D(const GtDocModel);
    return d->m_layoutMode;
}

void GtDocModel::setLayoutMode(LayoutMode mode)
{
    Q_D(GtDocModel);

    if (mode == d->m_layoutMode)
        return;

    d->m_layoutMode = mode;

    emit layoutModeChanged(d->m_layoutMode);
}

GtDocModel::SizingMode GtDocModel::sizingMode() const
{
    Q_D(const GtDocModel);
    return d->m_sizingMode;
}

void GtDocModel::setSizingMode(SizingMode mode)
{
    Q_D(GtDocModel);

    if (mode == d->m_sizingMode)
        return;

    d->m_sizingMode = mode;

    emit sizingModeChanged(d->m_sizingMode);
}

GtDocModel::MouseMode GtDocModel::mouseMode() const
{
    Q_D(const GtDocModel);
    return d->m_mouseMode;
}

void GtDocModel::setMouseMode(MouseMode mode)
{
    Q_D(GtDocModel);

    if (mode == d->m_mouseMode)
        return;

    d->m_mouseMode = mode;

    emit mouseModeChanged(d->m_mouseMode);
}

void GtDocModel::loadOutline()
{
    Q_D(GtDocModel);

    if (!d->m_document || !d->m_bookmarks) {
        qWarning() << "loadOutline document or bookmarks is null";
        return;
    }

    if (!d->m_document->isLoaded()) {
        qWarning() << "loadOutline document not loaded";
        return;
    }

    if (d->m_bookmarks->root()->children().size() > 0) {
        qWarning() << "loadOutline bookmarks is not empty";
        return;
    }

    d->m_document->loadOutline(d->m_bookmarks->root());
}

GT_END_NAMESPACE
