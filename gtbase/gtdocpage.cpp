/*
 * Copyright (C) 2013 Tom Wong. All rights reserved.
 */
#include "gtdocpage_p.h"

GT_BEGIN_NAMESPACE

GtDocPagePrivate::GtDocPagePrivate()
    : index(-1)
{
}

GtDocPagePrivate::~GtDocPagePrivate()
{
}

GtDocPage::GtDocPage(QObject *parent)
    : QObject(parent)
    , d_ptr(new GtDocPagePrivate())
{
    d_ptr->q_ptr = this;
}

GtDocPage::GtDocPage(GtDocPagePrivate &dd, QObject *parent)
    : QObject(parent)
    , d_ptr(&dd)
{
    d_ptr->q_ptr = this;
}

GtDocPage::~GtDocPage()
{
}

int GtDocPage::index()
{
    Q_D(GtDocPage);
    return d->index;
}

GT_END_NAMESPACE
