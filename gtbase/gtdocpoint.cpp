/*
 * Copyright (C) 2013 Tom Wong. All rights reserved.
 */
#include "gtdocpoint.h"
#include "gtdocpage.h"
#include <QtCore/QDebug>
#include <QtCore/QPoint>
#include <QtCore/QRect>
#include <math.h>

GT_BEGIN_NAMESPACE

GtDocPoint::GtDocPoint(GtDocPage *page, const QPointF &point)
    : _page(page)
    , _text(-1)
    , _x(point.x())
    , _y(point.y())
{
}

bool GtDocPoint::isValid() const
{
    if (_page) {
        qreal w, h;
        _page->size(&w, &h);
        return _x >= 0 && _x <= w && _y >= 0 && _y <= h;
    }

    return false;
}

int GtDocPoint::text(bool inside) const
{
    if (_text != -1)
        return _text;

    if (!_page)
        return -1;

    int i, result = -1;

    GtDocTextPointer text(_page->text());
    const QRectF *rect = text->rects();

    if (inside) {
        for (i = 0; i < text->length(); ++i, ++rect) {
            if (_x >= rect->left() && _x < rect->right() &&
                _y >= rect->top() && _y < rect->bottom())
            {
                result = i;
                *(int*)&_text = i;
                break;
            }
        }
    }
    else {
        double dist, maxDist = -1;

        for (i = 0; i < text->length(); ++i, ++rect) {
            dist = hypot(_x - rect->x() - rect->width() / 2.0,
                         _y - rect->y() - rect->height() / 2.0);
            if (maxDist < 0 || dist < maxDist) {
                maxDist = dist;
                result = i;
            }
        }

        // check if point is inside right half of the char
        if (result != -1) {
            rect = text->rects() + result;

            if (_x > rect->x() + rect->width() / 2.0)
                ++result;
        }
    }

    return result;
}

GtDocPoint GtDocPoint::normalized() const
{
    if (_page) {
        qreal w, h;
        _page->size(&w, &h);
        return GtDocPoint(_page, CLAMP(_x, 0, w), CLAMP(_y, 0, h));
    }

    return *this;
}

GtDocPoint GtDocPoint::startOfWord() const
{
    return GtDocPoint();
}

GtDocPoint GtDocPoint::endOfWord() const
{
    return GtDocPoint();
}

GtDocPoint GtDocPoint::startOfLine() const
{
    return GtDocPoint();
}

GtDocPoint GtDocPoint::endOfLine() const
{
    return GtDocPoint();
}

bool operator>(const GtDocPoint &p1, const GtDocPoint &p2)
{
    int index1 = p1.page() ? p1.page()->index() : -1;
    int index2 = p2.page() ? p2.page()->index() : -1;

    if (index1 > index2)
        return true;

    if (index1 != index2)
        return false;

    if (p1.y() > p2.y())
        return true;

    if (p1.y() == p2.y() && p1.x() > p2.x())
        return true;

    return false;
}

bool operator<(const GtDocPoint &p1, const GtDocPoint &p2)
{
    int index1 = p1.page() ? p1.page()->index() : -1;
    int index2 = p2.page() ? p2.page()->index() : -1;

    if (index1 < index2)
        return true;

    if (index1 != index2)
        return false;

    if (p1.y() < p2.y())
        return true;

    if (p1.y() == p2.y() && p1.x() < p2.x())
        return true;

    return false;
}

bool operator>=(const GtDocPoint &p1, const GtDocPoint &p2)
{
    int index1 = p1.page() ? p1.page()->index() : -1;
    int index2 = p2.page() ? p2.page()->index() : -1;

    if (index1 > index2)
        return true;

    if (index1 != index2)
        return false;

    if (p1.y() > p2.y())
        return true;

    if (p1.y() == p2.y() && p1.x() >= p2.x())
        return true;

    return false;
}

bool operator<=(const GtDocPoint &p1, const GtDocPoint &p2)
{
    int index1 = p1.page() ? p1.page()->index() : -1;
    int index2 = p2.page() ? p2.page()->index() : -1;

    if (index1 < index2)
        return true;

    if (index1 != index2)
        return false;

    if (p1.y() < p2.y())
        return true;

    if (p1.y() == p2.y() && p1.x() <= p2.x())
        return true;

    return false;
}

#ifndef QT_NO_DEBUG_STREAM
QDebug operator<<(QDebug dbg, const GtDocPoint &r) {
    int index = r.page() ? r.page()->index() : -1;
    dbg.nospace() << "GtDocPoint(" << index << ' ' << r.x() << ',' << r.y() << ')';
    return dbg.space();
}
#endif

GT_END_NAMESPACE
