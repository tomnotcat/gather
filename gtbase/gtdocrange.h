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
    Q_DECL_CONSTEXPR GtDocRange() : _textBegin(-1), _textEnd(-1) {}
    inline GtDocRange(const GtDocPoint &begin, const GtDocPoint &end);
    inline GtDocRange(const GtDocRange &other);

    inline bool isEmpty() const;

    inline void setPoints(const GtDocPoint &begin, const GtDocPoint &end);
    QPoint intersectedText(GtDocPage *page);

private:
    GtDocPoint _begin;
    GtDocPoint _end;
    int _textBegin;
    int _textEnd;
};

inline GtDocRange::GtDocRange(const GtDocPoint &begin, const GtDocPoint &end)
    : _begin(begin), _end(end), _textBegin(-1), _textEnd(-1) {}

inline bool GtDocRange::isEmpty() const
{ return _begin <= _end; }

inline void GtDocRange::setPoints(const GtDocPoint &begin, const GtDocPoint &end)
{ _begin = begin; _end = end; _textBegin = -1; _textEnd = -1; }

GT_END_NAMESPACE

#endif  /* __GT_DOC_RANGE_H__ */
