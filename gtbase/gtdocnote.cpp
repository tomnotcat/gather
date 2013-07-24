/*
 * Copyright (C) 2013 Tom Wong. All rights reserved.
 */
#include "gtdocnote_p.h"
#include <QtCore/QDebug>

GT_BEGIN_NAMESPACE

GtDocNotePrivate::GtDocNotePrivate(GtDocNote *q)
    : q_ptr(q)
{
}

GtDocNotePrivate::~GtDocNotePrivate()
{
}

GtDocNote::GtDocNote(QObject *parent)
    : QObject(parent)
    , d_ptr(new GtDocNotePrivate(this))
{
}

GtDocNote::GtDocNote(const GtDocRange &range, QObject *parent)
    : QObject(parent)
    , d_ptr(new GtDocNotePrivate(this))
{
    setRange(range);
}

GtDocNote::GtDocNote(GtDocNotePrivate &dd, QObject* parent)
    : QObject(parent)
    , d_ptr(&dd)
{
}

GtDocNote::~GtDocNote()
{
}

GtDocRange GtDocNote::range() const
{
    Q_D(const GtDocNote);
    return d->range;
}

void GtDocNote::setRange(const GtDocRange &range)
{
    Q_ASSERT(range.type() != GtDocRange::UnknownRange);

    Q_D(GtDocNote);
    d->range = range;
}

QString GtDocNote::text() const
{
    Q_D(const GtDocNote);
    return d->text;
}

void GtDocNote::setText(const QString &text)
{
    Q_D(GtDocNote);
    d->text = text;
}

GT_END_NAMESPACE
