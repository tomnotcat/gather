/*
 * Copyright (C) 2013 Tom Wong. All rights reserved.
 */
#include "gtbookmark.h"
#include "gtdocmessage.pb.h"
#include "gtserialize.h"
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

void GtBookmark::remove(GtBookmark *bookmark)
{
    Q_ASSERT(bookmark->m_parent == this);

    if (m_children.removeOne(bookmark))
        bookmark->m_parent = 0;
}

void GtBookmark::clearChildren()
{
    qDeleteAll(m_children);
    m_children.clear();
}

void GtBookmark::serialize(GtBookmarkMsg &msg) const
{
    msg.set_title(m_title.toUtf8());
    m_dest.serialize(*msg.mutable_dest());

    QList<GtBookmark*>::const_iterator it;
    for (it = m_children.begin(); it != m_children.end(); ++it)
        (*it)->serialize(*msg.add_children());
}

bool GtBookmark::deserialize(const GtBookmarkMsg &msg)
{
    setTitle(msg.title().c_str());
    m_dest.deserialize(msg.dest());

    int count = msg.children_size();
    for (int i = 0; i < count; ++i) {
        GtBookmark *p = new GtBookmark();

        if (p->deserialize(msg.children(i))) {
            append(p);
        }
        else {
            delete p;
            return false;
        }
    }

    return true;
}

#ifndef QT_NO_DATASTREAM

QDataStream &operator<<(QDataStream &s, const GtBookmark &b)
{
    return GtSerialize::serialize<GtBookmark, GtBookmarkMsg>(s, b);
}

QDataStream &operator>>(QDataStream &s, GtBookmark &b)
{
    return GtSerialize::deserialize<GtBookmark, GtBookmarkMsg>(s, b);
}

#endif // QT_NO_DATASTREAM

GT_END_NAMESPACE
