/*
 * Copyright (C) 2013 Tom Wong. All rights reserved.
 */
#ifndef __GT_DOC_RANGE_H__
#define __GT_DOC_RANGE_H__

#include "gtdocpoint.h"
#include <QtCore/QPoint>

GT_BEGIN_NAMESPACE

class GtDocRangeMsg;

class GT_BASE_EXPORT GtDocRange : public GtObject
{
public:
    enum RangeType {
        UnknownRange,
        TextRange,
        GeomRange
    };

public:
    Q_DECL_CONSTEXPR GtDocRange() : m_type(UnknownRange) {}
    inline GtDocRange(const GtDocPoint &begin,
                      const GtDocPoint &end,
                      RangeType type = UnknownRange);
    inline GtDocPoint begin() const { return m_begin; }
    inline GtDocPoint end() const { return m_end; }
    inline RangeType type() const { return m_type; }
    inline bool isEmpty() const;

    inline void setRange(const GtDocPoint &begin, const GtDocPoint &end);
    inline void setType(RangeType type) { m_type = type; }
    inline bool contains(const GtDocPoint &point) const;
    QPoint intersectedText(GtDocPage *page) const;

    void serialize(GtDocRangeMsg &msg) const;
    bool deserialize(const GtDocRangeMsg &msg);

private:
    GtDocPoint m_begin;
    GtDocPoint m_end;
    RangeType m_type;
};

inline GtDocRange::GtDocRange(const GtDocPoint &begin,
                              const GtDocPoint &end,
                              GtDocRange::RangeType type)
    : m_begin(begin), m_end(end), m_type(type) {}

inline bool GtDocRange::isEmpty() const
{ return m_begin.isNull() || m_end.isNull() || m_end <= m_begin; }

inline void GtDocRange::setRange(const GtDocPoint &begin, const GtDocPoint &end)
{ m_begin = begin; m_end = end; }

inline bool GtDocRange::contains(const GtDocPoint &p) const
{ return !isEmpty() && !p.isNull() && p >= m_begin && p <= m_end; }

#ifndef QT_NO_DATASTREAM
GT_BASE_EXPORT QDataStream &operator<<(QDataStream &, const GtDocRange &);
GT_BASE_EXPORT QDataStream &operator>>(QDataStream &, GtDocRange &);
#endif

GT_END_NAMESPACE

#endif  /* __GT_DOC_RANGE_H__ */
