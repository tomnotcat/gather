/*
 * Copyright (C) 2013 Tom Wong. All rights reserved.
 */
#include "gttabview.h"
#include "gtmainwindow.h"
#include <QtCore/QDebug>

GT_BEGIN_NAMESPACE

GtTabView::GtTabView(QWidget *parent)
    : QWidget(parent)
    , m_active(false)
{
    m_mainWindow = qobject_cast<GtMainWindow*>(parent);
}

GtTabView::~GtTabView()
{
}

void GtTabView::gainActive()
{
    Q_ASSERT(!m_active);
    m_active = true;
}

void GtTabView::loseActive()
{
    Q_ASSERT(m_active);
    m_active = false;
}

void GtTabView::saveSettings(GtMainSettings *settings)
{
    Q_UNUSED(settings);
}

GT_END_NAMESPACE
