/*
 * Copyright (C) 2013 Tom Wong. All rights reserved.
 */
#include "gtdocoutline.h"
#include "gtabstractdocument.h"
#include <QtCore/QDebug>

GT_BEGIN_NAMESPACE

class GtDocOutlinePrivate
{
    Q_DECLARE_PUBLIC(GtDocOutline)

public:
    GtDocOutlinePrivate();
    ~GtDocOutlinePrivate();

protected:
    void loadOutline(GtAbstractOutline *ao, GtDocOutline::Entry **pe, void *it);
    void freeEntry(GtDocOutline::Entry *e);

protected:
    GtDocOutline *q_ptr;
    GtDocOutline::Entry *entry;
};

GtDocOutlinePrivate::GtDocOutlinePrivate()
    : q_ptr(0)
    , entry(0)
{
}

GtDocOutlinePrivate::~GtDocOutlinePrivate()
{
    freeEntry(entry);
}

void GtDocOutlinePrivate::loadOutline(GtAbstractOutline *ao,
                                      GtDocOutline::Entry **pe,
                                      void *it)
{
    while (it) {
        *pe = new GtDocOutline::Entry(ao->title(it), ao->page(it));

        void *child = ao->childIterator(it);
        if (child) {
            loadOutline(ao, &(*pe)->child, child);
            ao->freeIterator(child);
        }

        it = ao->nextIterator(it);
        pe = &(*pe)->next;
    }
}

void GtDocOutlinePrivate::freeEntry(GtDocOutline::Entry *e)
{
    GtDocOutline::Entry *n;

    while (e) {
        n = e->next;
        if (e->child)
            freeEntry(e->child);

        delete e;
        e = n;
    }
}

GtDocOutline::Iterator::Iterator(const GtDocOutline &o, const GtDocOutline::Entry *e)
    : outline(&o)
    , entry(e)
{
    if (0 == entry)
        entry = outline->d_ptr->entry;
}

GtDocOutline::Iterator::~Iterator()
{
}

GtDocOutline::Entry GtDocOutline::Iterator::next()
{
    const Entry *r = entry;
    entry = entry->next;
    return *r;
}

GtDocOutline::GtDocOutline(GtAbstractOutline *ao, QObject *parent)
    : QObject(parent)
    , d_ptr(new GtDocOutlinePrivate())
{
    d_ptr->q_ptr = this;

    QScopedPointer<GtAbstractOutline> guard(ao);
    void *it = ao->iterator();
    d_ptr->loadOutline(ao, &d_ptr->entry, it);
    ao->freeIterator(it);
}

GtDocOutline::~GtDocOutline()
{
}

#ifndef QT_NO_DEBUG_STREAM
QDebug operator<<(QDebug dbg, const GtDocOutline::Entry &r) {
    dbg.nospace() << "GtDocOutline::Entry(" << r.title << ':' << r.page << ')';
    return dbg.space();
}
#endif

GT_END_NAMESPACE
