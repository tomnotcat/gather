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

GtBookmark::GtBookmark(const GtLinkDest &dest)
    : m_dest(dest)
    , m_parent(0)
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

GtBookmark* GtBookmark::prev()
{
    if (!m_parent)
        return 0;

    int index = m_parent->m_children.indexOf(this);
    if (index > 0)
        return m_parent->m_children[index - 1];

    return 0;
}

GtBookmark* GtBookmark::next()
{
    if (!m_parent)
        return 0;

    int index = m_parent->m_children.indexOf(this);
    if (index < m_parent->m_children.size() - 1)
        return m_parent->m_children[index + 1];

    return 0;
}

void GtBookmark::insert(GtBookmark *before, GtBookmark *bookmark)
{
    Q_ASSERT(!before || before->m_parent == this);
    Q_ASSERT(bookmark && !bookmark->m_parent);

    bookmark->m_parent = this;

    if (before) {
        m_children.insert(m_children.indexOf(before), bookmark);
    }
    else {
        m_children.push_back(bookmark);
    }
}

void GtBookmark::clearChildren()
{
    qDeleteAll(m_children);
    m_children.clear();
}

GT_END_NAMESPACE
