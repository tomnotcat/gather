/*
 * Copyright (C) 2013 Tom Wong. All rights reserved.
 */
#include "gtlinkdest.h"
#include <QtCore/QDebug>

GT_BEGIN_NAMESPACE

GtLinkDest::GtLinkDest()
    : m_type(LinkNone)
    , m_page(-1)
    , m_zoom(0)
{
}

GtLinkDest::GtLinkDest(int page, const QPointF &point, double zoom)
    : m_type(ScrollTo)
    , m_page(page)
    , m_point(point)
    , m_zoom(zoom)
{
}

GtLinkDest::~GtLinkDest()
{
}

GT_END_NAMESPACE
