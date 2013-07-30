/*
 * Copyright (C) 2013 Tom Wong. All rights reserved.
 */
#ifndef __GT_DOC_POINT_H__
#define __GT_DOC_POINT_H__

#include "gtobject.h"
#include <QtCore/QPointF>

class QChar;

GT_BEGIN_NAMESPACE

class GtDocPage;

class GT_BASE_EXPORT GtDocPoint : public GtObject
{
public:
    Q_DECL_CONSTEXPR GtDocPoint() : m_page(0), m_text(-1) {}
    GtDocPoint(GtDocPage *page, const QPointF &point);
    GtDocPoint(GtDocPage *page, int text);
    GtDocPoint(GtDocPage *page, qreal x, qreal y) : m_page(page), m_point(x, y), m_text(-1) {}

    inline bool isNull() const;
    bool isValid() const;
    void normalize();

    inline GtDocPage* page() const;
    inline QPointF point() const;
    inline qreal x() const;
    inline qreal y() const;
    int text(bool inside) const;
    GtDocPoint beginOfWord(bool inside) const;
    GtDocPoint endOfWord(bool inside) const;

    inline void setPage(GtDocPage *page);
    inline void moveTo(qreal x, qreal y);
    inline void setX(qreal x);
    inline void setY(qreal y);

    friend bool operator==(const GtDocPoint &, const GtDocPoint &);
    friend bool operator!=(const GtDocPoint &, const GtDocPoint &);
    friend bool operator>(const GtDocPoint &, const GtDocPoint &);
    friend bool operator<(const GtDocPoint &, const GtDocPoint &);
    friend bool operator>=(const GtDocPoint &, const GtDocPoint &);
    friend bool operator<=(const GtDocPoint &, const GtDocPoint &);

public:
    static bool isSpace(const QChar &c);
    static bool isWordSeparator(const QChar &c);

private:
    GtDocPage *m_page;
    QPointF m_point;
    int m_text;
};

inline bool GtDocPoint::isNull() const
{ return m_page == 0 && m_point.isNull(); }

inline GtDocPage* GtDocPoint::page() const
{ return m_page; }

inline QPointF GtDocPoint::point() const
{ return m_point; }

inline qreal GtDocPoint::x() const
{ return m_point.x(); }

inline qreal GtDocPoint::y() const
{ return m_point.y(); }

inline void GtDocPoint::setPage(GtDocPage *page)
{ m_page = page; }

inline void GtDocPoint::moveTo(qreal x, qreal y)
{ m_point.setX(x); m_point.setY(y); }

inline void GtDocPoint::setX(qreal x)
{ m_point.setX(x); }

inline void GtDocPoint::setY(qreal y)
{ m_point.setY(y); }

#ifndef QT_NO_DEBUG_STREAM
GT_BASE_EXPORT QDebug operator<<(QDebug, const GtDocPoint &);
#endif

GT_END_NAMESPACE

#endif  /* __GT_DOC_POINT_H__ */
