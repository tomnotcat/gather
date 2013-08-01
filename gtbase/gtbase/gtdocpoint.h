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
class GtDocument;

class GT_BASE_EXPORT GtDocPoint : public GtObject
{
public:
    Q_DECL_CONSTEXPR GtDocPoint() : m_page(-1), m_text(-1) {}
    GtDocPoint(int page, const QPointF &point);
    GtDocPoint(GtDocPage *page, int text);
    GtDocPoint(int page, qreal x, qreal y) : m_page(page), m_text(-1), m_point(x, y) {}

    inline bool isNull() const;
    inline int page() const { return m_page; }
    inline QPointF point() const { return m_point; }
    inline qreal x() const { return m_point.x(); }
    inline qreal y() const { return m_point.y(); }
    int text(GtDocument *document, bool inside) const;
    GtDocPoint beginOfWord(GtDocument *document, bool inside) const;
    GtDocPoint endOfWord(GtDocument *document, bool inside) const;

    inline void setPage(int page) { m_page = page; }
    inline void setX(qreal x) { m_point.setX(x); }
    inline void setY(qreal y) { m_point.setY(y); }

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
    int m_page;
    int m_text;
    QPointF m_point;
};

inline bool GtDocPoint::isNull() const
{ return m_page == -1 && m_text == -1 && m_point.isNull(); }

#ifndef QT_NO_DEBUG_STREAM
GT_BASE_EXPORT QDebug operator<<(QDebug, const GtDocPoint &);
#endif

GT_END_NAMESPACE

#endif  /* __GT_DOC_POINT_H__ */
