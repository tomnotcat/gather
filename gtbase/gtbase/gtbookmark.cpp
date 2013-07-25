/*
 * Copyright (C) 2013 Tom Wong. All rights reserved.
 */
#include "gtbookmark.h"
#include <QtCore/QDebug>

GT_BEGIN_NAMESPACE

GtBookmark::GtBookmark()
    : m_parent(0)
{
}

GtBookmark::GtBookmark(const QString &title, const GtLinkDest &dest)
    : m_title(title)
    , m_dest(dest)
    , m_parent(0)
{
}

GtBookmark::~GtBookmark()
{
    qDeleteAll(m_children);
}

GtBookmark* GtBookmark::addChild(const QString &title, const GtLinkDest &dest)
{
    GtBookmark *child = new GtBookmark(title, dest);
    child->m_parent = this;
    m_children.push_back(child);
    return child;
}

GT_END_NAMESPACE
