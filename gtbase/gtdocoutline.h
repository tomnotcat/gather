/*
 * Copyright (C) 2013 Tom Wong. All rights reserved.
 */
#ifndef __GT_DOC_OUTLINE_H__
#define __GT_DOC_OUTLINE_H__

#include "gtcommon.h"

GT_BEGIN_NAMESPACE

class GtDocOutlinePrivate;

class GT_BASE_EXPORT GtDocOutline
{
public:
    GtDocOutline(const QString &title, int page);
    ~GtDocOutline();

public:
    GtDocOutline* parent();
    const GtDocOutline* parent() const;
    QList<GtDocOutline>& children();
    const QList<GtDocOutline>& children() const;
    const QString& title() const;
    int page() const;

protected:
    QScopedPointer<GtDocOutlinePrivate> d_ptr;

private:
    Q_DISABLE_COPY(GtDocOutline)
    Q_DECLARE_PRIVATE(GtDocOutline)
};

GT_END_NAMESPACE

#endif  /* __GT_DOC_OUTLINE_H__ */
