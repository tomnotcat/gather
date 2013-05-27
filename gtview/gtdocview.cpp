/*
 * Copyright (C) 2013 Tom Wong. All rights reserved.
 */
#include "gtdocview.h"

GT_BEGIN_NAMESPACE

class GtDocViewPrivate
{
    Q_DECLARE_PUBLIC(GtDocView)

public:
    GtDocViewPrivate();

private:
    GtDocView *q_ptr;
};

GtDocViewPrivate::GtDocViewPrivate()
{
}

GtDocView::GtDocView(QWidget *parent, Qt::WindowFlags f)
    : QWidget(parent, f)
    , d_ptr(new GtDocViewPrivate())
{
    d_ptr->q_ptr = this;
}

GtDocView::GtDocView(GtDocViewPrivate &dd, QWidget *parent, Qt::WindowFlags f)
    : QWidget(parent, f)
    , d_ptr(&dd)
{
    d_ptr->q_ptr = this;
}

GtDocView::~GtDocView()
{
}

GT_END_NAMESPACE
