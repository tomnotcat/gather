/*
 * Copyright (C) 2013 Tom Wong. All rights reserved.
 */
#include "gtdocpage_p.h"

GT_BEGIN_NAMESPACE

GtDocPagePrivate::GtDocPagePrivate()
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

GT_END_NAMESPACE
