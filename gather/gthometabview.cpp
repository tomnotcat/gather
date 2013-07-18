/*
 * Copyright (C) 2013 Tom Wong. All rights reserved.
 */
#include "gthometabview.h"

GT_BEGIN_NAMESPACE

GtHomeTabView::GtHomeTabView(QWidget *parent)
    : GtTabView(parent)
{
    ui.setupUi(this);
}

GtHomeTabView::~GtHomeTabView()
{
}

GT_END_NAMESPACE
