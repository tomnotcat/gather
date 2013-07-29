/*
 * Copyright (C) 2013 Tom Wong. All rights reserved.
 */
#include "gtlinkdest.h"
#include <QtCore/QDebug>

GT_BEGIN_NAMESPACE

GtLinkDest::GtLinkDest()
    : m_type(LinkNone)
{
}

GtLinkDest::GtLinkDest(int page, const QPointF &point, double zoom)
    : m_type(LinkGoto)
{
    m_data.gotor.page = page;
    m_data.gotor.x = point.x();
    m_data.gotor.y = point.y();
    m_data.gotor.zoom = zoom;
}

GtLinkDest::~GtLinkDest()
{
}

GT_END_NAMESPACE
