/*
 * Copyright (C) 2013 Tom Wong. All rights reserved.
 */
#include "gttabview.h"

GT_BEGIN_NAMESPACE

GtTabView::GtTabView(QWidget *parent)
    : QWidget(parent)
    , m_ui(0)
{
}

GtTabView::~GtTabView()
{
}

void GtTabView::currentChanged(GtTabView *old, GtTabView *now)
{
}

void GtTabView::onCut()
{
}

void GtTabView::onCopy()
{
}

void GtTabView::onPaste()
{
}

void GtTabView::onDelete()
{
}

void GtTabView::onZoomIn()
{
}

void GtTabView::onZoomOut()
{
}

void GtTabView::onRotateLeft()
{
}

void GtTabView::onRotateRight()
{
}

GT_END_NAMESPACE
