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

GtLinkDest::GtLinkDest(int page)
    : m_type(LinkGoto)
{
    m_data.gotor.page = page;
}

GtLinkDest::~GtLinkDest()
{
}

GT_END_NAMESPACE
