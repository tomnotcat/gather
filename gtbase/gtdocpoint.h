/*
 * Copyright (C) 2013 Tom Wong. All rights reserved.
 */
#ifndef __GT_DOC_POINT_H__
#define __GT_DOC_POINT_H__

#include "gtcommon.h"

class QPointF;

GT_BEGIN_NAMESPACE

class GtDocPage;

class GT_BASE_EXPORT GtDocPoint
{
public:
    Q_DECL_CONSTEXPR GtDocPoint() : _page(0), _x(0), _y(0) {}
    GtDocPoint(GtDocPage *page, const QPointF &point);
    GtDocPoint(GtDocPage *page, qreal x, qreal y) : _page(page), _x(x), _y(y) {}

    inline bool isNull() const;
    bool isValid() const;

    inline GtDocPage* page() const;
    inline qreal x() const;
    inline qreal y() const;
    int offset(bool inside) const;
    GtDocPoint normalized() const;

    inline void setPage(GtDocPage *page);
    inline void moveTo(qreal x, qreal y);
    inline void setX(qreal x);
    inline void setY(qreal y);

    friend inline bool operator==(const GtDocPoint &, const GtDocPoint &);
    friend inline bool operator!=(const GtDocPoint &, const GtDocPoint &);
    friend bool operator>(const GtDocPoint &, const GtDocPoint &);
    friend bool operator<(const GtDocPoint &, const GtDocPoint &);
    friend bool operator>=(const GtDocPoint &, const GtDocPoint &);
    friend bool operator<=(const GtDocPoint &, const GtDocPoint &);

private:
    GtDocPage *_page;
    qreal _x;
    qreal _y;
};

inline bool GtDocPoint::isNull() const
{ return _page == 0 && _x == 0 && _y == 0; }

inline GtDocPage* GtDocPoint::page() const
{ return _page; }

inline qreal GtDocPoint::x() const
{ return _x; }

inline qreal GtDocPoint::y() const
{ return _y; }

inline void GtDocPoint::setPage(GtDocPage *page)
{ _page = page; }

inline void GtDocPoint::moveTo(qreal x, qreal y)
{ _x = x; _y = y; }

inline void GtDocPoint::setX(qreal x)
{ _x = x; }

inline void GtDocPoint::setY(qreal y)
{ _y = y; }

inline bool operator==(const GtDocPoint &p1, const GtDocPoint &p2)
{ return p1._page == p2._page && p1._x == p2._x && p1._y == p2._y; }

inline bool operator!=(const GtDocPoint &p1, const GtDocPoint &p2)
{ return p1._page != p2._page || p1._x != p2._x || p1._y != p2._y; }

#ifndef QT_NO_DEBUG_STREAM
GT_BASE_EXPORT QDebug operator<<(QDebug, const GtDocPoint &);
#endif

GT_END_NAMESPACE

#endif  /* __GT_DOC_POINT_H__ */