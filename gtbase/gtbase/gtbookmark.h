/*
 * Copyright (C) 2013 Tom Wong. All rights reserved.
 */
#ifndef __GT_BOOKMARK_H__
#define __GT_BOOKMARK_H__

#include "gtlinkdest.h"
#include <QtCore/QList>
#include <QtCore/QString>

GT_BEGIN_NAMESPACE

class GT_BASE_EXPORT GtBookmark : public GtObject
{
public:
    explicit GtBookmark();
    GtBookmark(const QString &title, const GtLinkDest &dest);
    ~GtBookmark();

public:
    inline QString title() const { return m_title; }
    inline GtLinkDest dest() const { return m_dest; }
    inline GtBookmark* parent() const { return m_parent; }
    inline int index() { return m_parent ? m_parent->m_children.indexOf(this) : 0; }
    inline QList<GtBookmark*> children() const { return m_children; }
    GtBookmark* addChild(const QString &title, const GtLinkDest &dest);

private:
    QString m_title;
    GtLinkDest m_dest;
    GtBookmark *m_parent;
    QList<GtBookmark*> m_children;

private:
    Q_DISABLE_COPY(GtBookmark)
};

GT_END_NAMESPACE

#endif  /* __GT_BOOKMARK_H__ */
