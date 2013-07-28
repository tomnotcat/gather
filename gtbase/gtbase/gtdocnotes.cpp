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
    QList<GtDocNote*> m_notes;
    QVector<QList<GtDocNote*> > m_pages;
};

GtDocNotesPrivate::GtDocNotesPrivate(GtDocNotes *q)
    : q_ptr(q)
{
}

GtDocNotesPrivate::~GtDocNotesPrivate()
{
    qDeleteAll(m_notes);
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
    return d->m_pages.size();
}

QList<GtDocNote*> GtDocNotes::pageNotes(int page) const
{
    Q_D(const GtDocNotes);

    if (page < d->m_pages.size())
        return d->m_pages[page];

    return QList<GtDocNote*>();
}

QList<GtDocNote*> GtDocNotes::allNotes() const
{
    Q_D(const GtDocNotes);
    return d->m_notes;
}

void GtDocNotes::addNote(GtDocNote *note)
{
    Q_D(GtDocNotes);

    // add to list
    d->m_notes.push_back(note);

    // add to pages
    GtDocRange range(note->range());
    int beginPage = range.begin().page()->index();
    int endPage = range.end().page()->index() + 1;

    if (endPage > d->m_pages.size())
        d->m_pages.resize(endPage);

    for (int i = beginPage; i < endPage; ++i)
        d->m_pages[i].push_back(note);

    emit noteAdded(note);
}

bool GtDocNotes::removeNote(GtDocNote *note)
{
    Q_D(GtDocNotes);

    // remove from list
    if (!d->m_notes.removeOne(note))
        return false;

    // remove from pages
    GtDocRange range(note->range());
    int beginPage = range.begin().page()->index();
    int endPage = range.end().page()->index() + 1;

    for (int i = beginPage; i < endPage; ++i)
        d->m_pages[i].removeOne(note);

    while (d->m_pages.size() > 0) {
        if (d->m_pages.last().size() == 0)
            d->m_pages.pop_back();
        else
            break;
    }

    emit noteRemoved(note);
    return true;
}

GT_END_NAMESPACE
