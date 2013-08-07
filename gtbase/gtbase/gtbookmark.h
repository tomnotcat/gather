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
    enum UpdateFlag {
        UpdateNone    = 0x00000000,
        UpdateTitle   = 0x00000001,
        UpdateDest    = 0x00000002
    };

public:
    explicit GtBookmark();
    explicit GtBookmark(const GtLinkDest &dest);
    GtBookmark(const QString &title, const GtLinkDest &dest);
    ~GtBookmark();

public:
    inline QString title() const { return m_title; }
    inline void setTitle(const QString &title) { m_title = title; }

    inline GtLinkDest dest() const { return m_dest; }
    inline void setDest(const GtLinkDest &dest) { m_dest = dest; }

    inline GtBookmark* parent() const { return m_parent; }
    inline int index() { return m_parent ? m_parent->m_children.indexOf(this) : 0; }
    inline QList<GtBookmark*> children() const { return m_children; }

    GtBookmark* prev();
    GtBookmark* next();

    void insert(GtBookmark *before, GtBookmark *bookmark);
    void remove(GtBookmark *bookmark);
    void clearChildren();
    inline void append(GtBookmark *bookmark) { insert(0, bookmark); }

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
