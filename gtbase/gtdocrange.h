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
    Q_DECL_CONSTEXPR GtDocRange() {}
    inline GtDocRange(const GtDocPoint &begin, const GtDocPoint &end);
    inline GtDocPoint begin() const { return _begin; }
    inline GtDocPoint end() const { return _end; }
    inline bool isEmpty() const;

    inline void setPoints(const GtDocPoint &begin, const GtDocPoint &end);
    inline bool contains(const GtDocPoint &point) const;
    QPoint intersectedText(GtDocPage *page);

private:
    GtDocPoint _begin;
    GtDocPoint _end;
};

inline GtDocRange::GtDocRange(const GtDocPoint &begin, const GtDocPoint &end)
    : _begin(begin), _end(end) {}

inline bool GtDocRange::isEmpty() const
{ return _begin.isNull() || _end.isNull() || _end <= _begin; }

inline void GtDocRange::setPoints(const GtDocPoint &begin, const GtDocPoint &end)
{ _begin = begin; _end = end; }

inline bool GtDocRange::contains(const GtDocPoint &p) const
{ return !isEmpty() && !p.isNull() && p >= _begin && p <= _end; }

GT_END_NAMESPACE

#endif  /* __GT_DOC_RANGE_H__ */



