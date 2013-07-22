/*
 * Copyright (C) 2013 Tom Wong. All rights reserved.
 */
#ifndef __GT_DOC_ANNOT_H__
#define __GT_DOC_ANNOT_H__

#include "gtobject.h"
#include <QtCore/QObject>

GT_BEGIN_NAMESPACE

class GtDocAnnotPrivate;

class GT_BASE_EXPORT GtDocAnnot : public QObject, public GtObject
{
    Q_OBJECT

public:
    explicit GtDocAnnot(QObject *parent = 0);
    ~GtDocAnnot();

protected:
    QScopedPointer<GtDocAnnotPrivate> d_ptr;

private:
    Q_DISABLE_COPY(GtDocAnnot)
    Q_DECLARE_PRIVATE(GtDocAnnot)
};

GT_END_NAMESPACE

#endif  /* __GT_DOC_ANNOT_H__ */
