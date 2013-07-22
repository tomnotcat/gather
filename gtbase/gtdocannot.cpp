/*
 * Copyright (C) 2013 Tom Wong. All rights reserved.
 */
#include "gtdocannot.h"
#include <QtCore/QDebug>

GT_BEGIN_NAMESPACE

class GtDocAnnotPrivate
{
    Q_DECLARE_PUBLIC(GtDocAnnot)

public:
    explicit GtDocAnnotPrivate(GtDocAnnot *q);
    ~GtDocAnnotPrivate();

protected:
    GtDocAnnot *q_ptr;
};

GtDocAnnotPrivate::GtDocAnnotPrivate(GtDocAnnot *q)
    : q_ptr(q)
{
}

GtDocAnnotPrivate::~GtDocAnnotPrivate()
{
}

GtDocAnnot::GtDocAnnot(QObject *parent)
    : QObject(parent)
    , d_ptr(new GtDocAnnotPrivate(this))
{
}

GtDocAnnot::~GtDocAnnot()
{
}

GT_END_NAMESPACE
