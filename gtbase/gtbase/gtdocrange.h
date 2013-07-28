/*
 * Copyright (C) 2013 Tom Wong. All rights reserved.
 */
#ifndef __GT_DOC_RANGE_H__
#define __GT_DOC_RANGE_H__

#include "gtdocpoint.h"
#include <QtCore/QPoint>

GT_BEGIN_NAMESPACE

class GT_BASE_EXPORT GtDocRange : public GtObject
{
public:
    enum RangeType {
        UnknownRange,
        TextRange,
        GeomRange
    };

public:
    Q_DECL_CONSTEXPR GtDocRange() : _type(UnknownRange) {}
    inline GtDocRange(const GtDocPoint &begin,
                      const GtDocPoint &end,
                      RangeType type = UnknownRange);
    inline GtDocPoint begin() const { return _begin; }
    inline GtDocPoint end() const { return _end; }
    inline RangeType type() const { return _type; }
    inline bool isEmpty() const;

    inline void setRange(const GtDocPoint &begin, const GtDocPoint &end);
    inline void setType(RangeType type) { _type = type; }
    inline bool contains(const GtDocPoint &point) const;
    QPoint intersectedText(GtDocPage *page) const;

private:
    GtDocPoint _begin;
    GtDocPoint _end;
    RangeType _type;
};

inline GtDocRange::GtDocRange(const GtDocPoint &begin,
                              const GtDocPoint &end,
                              GtDocRange::RangeType type)
    : _begin(begin), _end(end), _type(type) {}

inline bool GtDocRange::isEmpty() const
{ return _begin.isNull() || _end.isNull() || _end <= _begin; }

inline void GtDocRange::setRange(const GtDocPoint &begin, const GtDocPoint &end)
{ _begin = begin; _end = end; }

inline bool GtDocRange::contains(const GtDocPoint &p) const
{ return !isEmpty() && !p.isNull() && p >= _begin && p <= _end; }

GT_END_NAMESPACE

#endif  /* __GT_DOC_RANGE_H__ */
