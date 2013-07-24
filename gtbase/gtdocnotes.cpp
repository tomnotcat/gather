/*
 * Copyright (C) 2013 Tom Wong. All rights reserved.
 */
#include "gtdocnotes.h"
#include "gtdocnote.h"
#include "gtdocpage.h"
#include "gtdocrange.h"
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
    QList<GtDocNote*> notes;
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

QList<GtDocNote*> GtDocNotes::pageNotes(int page) const
{
    Q_D(const GtDocNotes);

    if (page < d->pages.size())
        return QList<GtDocNote*>();

    return d->pages[page];
}

QList<GtDocNote*> GtDocNotes::allNotes() const
{
    Q_D(const GtDocNotes);
    return d->notes;
}

void GtDocNotes::addNote(GtDocNote *note)
{
    Q_D(GtDocNotes);

    // add to list
    d->notes.push_back(note);

    // add to pages
    GtDocRange range(note->range());
    int beginPage = range.begin().page()->index();
    int endPage = range.end().page()->index() + 1;

    d->pages.resize(endPage);
    for (int i = beginPage; i < endPage; ++i)
        d->pages[i].push_back(note);
}

GT_END_NAMESPACE
