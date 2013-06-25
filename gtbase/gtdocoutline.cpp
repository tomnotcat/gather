/*
 * Copyright (C) 2013 Tom Wong. All rights reserved.
 */
#include "gtdocoutline.h"
#include <QtCore/QDebug>

GT_BEGIN_NAMESPACE

GtDocOutline::GtDocOutline()
    : page(-1)
    , row(0)
    , _next(0)
    , _parent(0)
    , _child(0)
    , _childCount(0)
{
}

GtDocOutline::GtDocOutline(const QString &t, int p, int r)
    : title(t)
    , page(p)
    , row(r)
    , _next(0)
    , _parent(0)
    , _child(0)
    , _childCount(0)
{
}

GtDocOutline::~GtDocOutline()
{
    GtDocOutline *n;
    GtDocOutline *p = _child;

    while (p) {
        n = p->_next;
        delete p;
        p = n;
    }
}

GtDocOutline* GtDocOutline::child(int row) const
{
    GtDocOutline *p = _child;

    for (int i = 0; i < row && p; ++i)
        p = p->_next;

    return p;
}

#ifndef QT_NO_DEBUG_STREAM
QDebug operator<<(QDebug dbg, const GtDocOutline &r) {
    dbg.nospace() << "GtDocOutline(" << r.title << ':' << r.page << ')';
    return dbg.space();
}
#endif

GT_END_NAMESPACE
