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
    void loadOutline(GtAbstractOutline *ao,
                     GtDocOutline::Node *parent,
                     GtDocOutline::Node **pn,
                     void *it);
    void freeNode(GtDocOutline::Node *node);

protected:
    GtDocOutline *q_ptr;
    GtDocOutline::Node *first;
};

GtDocOutline::Node* GtDocOutline::Node::child(int row) const
{
    return 0;
}

GtDocOutlinePrivate::GtDocOutlinePrivate()
    : q_ptr(0)
    , first(0)
{
}

GtDocOutlinePrivate::~GtDocOutlinePrivate()
{
    freeNode(first);
}

void GtDocOutlinePrivate::loadOutline(GtAbstractOutline *ao,
                                      GtDocOutline::Node *parent,
                                      GtDocOutline::Node **pn,
                                      void *it)
{
    int row = 0;
    GtDocOutline::Node **cur = pn;

    while (it) {
        *cur = new GtDocOutline::Node(ao->title(it), ao->page(it), row++);
        (*cur)->_parent = parent;

        void *child = ao->childNode(it);
        if (child) {
            loadOutline(ao, *cur, &(*cur)->_child, child);
            ao->freeNode(child);
        }

        it = ao->nextNode(it);
        cur = &(*cur)->_next;
    }

    if (row > 0)
        (*pn)->_childCount = row;
}

void GtDocOutlinePrivate::freeNode(GtDocOutline::Node *p)
{
    GtDocOutline::Node *n;

    while (p) {
        n = p->_next;
        if (p->_child)
            freeNode(p->_child);

        delete p;
        p = n;
    }
}

GtDocOutline::GtDocOutline(GtAbstractOutline *ao, QObject *parent)
    : QObject(parent)
    , d_ptr(new GtDocOutlinePrivate())
{
    d_ptr->q_ptr = this;

    QScopedPointer<GtAbstractOutline> guard(ao);
    void *it = ao->firstNode();
    d_ptr->loadOutline(ao, 0, &d_ptr->first, it);
    ao->freeNode(it);
}

GtDocOutline::~GtDocOutline()
{
}

GtDocOutline::Node* GtDocOutline::first() const
{
    Q_D(const GtDocOutline);
    return d->first;
}

#ifndef QT_NO_DEBUG_STREAM
QDebug operator<<(QDebug dbg, const GtDocOutline::Node &r) {
    dbg.nospace() << "GtDocOutline::Node(" << r.title << ':' << r.page << ')';
    return dbg.space();
}
#endif

GT_END_NAMESPACE
