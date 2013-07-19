/*
 * Copyright (C) 2013 Tom Wong. All rights reserved.
 */
#include "gttabview.h"
#include "gtmainwindow.h"

GT_BEGIN_NAMESPACE

GtTabView::GtTabView(QWidget *parent)
    : QWidget(parent)
{
    m_mainWindow = qobject_cast<GtMainWindow*>(parent);
}

GtTabView::~GtTabView()
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

void GtTabView::gainCurrent()
{
}

void GtTabView::loseCurrent()
{
}

void GtTabView::mainWindowClose(GtTabView *current)
{
    Q_UNUSED(current);
}

GT_END_NAMESPACE
