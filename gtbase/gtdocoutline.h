/*
 * Copyright (C) 2013 Tom Wong. All rights reserved.
 */
#ifndef __GT_DOC_OUTLINE_H__
#define __GT_DOC_OUTLINE_H__

#include "gtobject.h"
#include <QtCore/QString>

GT_BEGIN_NAMESPACE

class GT_BASE_EXPORT GtDocOutline : public GtObject
{
public:
    explicit GtDocOutline();
    GtDocOutline(const QString &title, int page, int row);
    ~GtDocOutline();

public:
    inline GtDocOutline* next() const { return _next; }
    inline GtDocOutline* parent() const { return _parent; }
    inline GtDocOutline* child() const { return _child; }
    inline int childCount() const { return _childCount; }
    GtDocOutline* child(int row) const;

public:
    const QString title;
    const int page;
    const int row;

private:
    friend class GtDocumentPrivate;
    GtDocOutline *_next;
    GtDocOutline *_parent;
    GtDocOutline *_child;
    int _childCount;

private:
    Q_DISABLE_COPY(GtDocOutline)
};

#ifndef QT_NO_DEBUG_STREAM
GT_BASE_EXPORT QDebug operator<<(QDebug, const GtDocOutline &);
#endif

GT_END_NAMESPACE

#endif  /* __GT_DOC_OUTLINE_H__ */
