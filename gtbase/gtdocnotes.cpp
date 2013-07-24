/*
 * Copyright (C) 2013 Tom Wong. All rights reserved.
 */
#include "gtdocnotes.h"
#include <QtCore/QDebug>

GT_BEGIN_NAMESPACE

class GtDocNotesPrivate
{
    Q_DECLARE_PUBLIC(GtDocNotes)

public:
    explicit GtDocNotesPrivate(GtDocNotes *q);
    ~GtDocNotesPrivate();

protected:
    GtDocNotes *q_ptr;
    QVector<QList<GtDocNote*> > pages;
};

GtDocNotesPrivate::GtDocNotesPrivate(GtDocNotes *q)
    : q_ptr(q)
{
}

GtDocNotesPrivate::~GtDocNotesPrivate()
{
}

GtDocNotes::GtDocNotes(QObject *parent)
    : QObject(parent)
    , d_ptr(new GtDocNotesPrivate(this))
{
}

GtDocNotes::~GtDocNotes()
{
}

int GtDocNotes::pageCount() const
{
    Q_D(const GtDocNotes);
    return d->pages.size();
}

QList<GtDocNote*> GtDocNotes::pageNotes(int index) const
{
    Q_D(const GtDocNotes);

    if (index < d->pages.size())
        return QList<GtDocNote*>();

    return d->pages[index];
}

GT_END_NAMESPACE
