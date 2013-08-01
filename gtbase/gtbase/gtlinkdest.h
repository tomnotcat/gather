/*
 * Copyright (C) 2013 Tom Wong. All rights reserved.
 */
#ifndef __GT_LINK_DEST_H__
#define __GT_LINK_DEST_H__

#include "gtobject.h"
#include <QtCore/QPointF>
#include <QtCore/QString>

GT_BEGIN_NAMESPACE

class GT_BASE_EXPORT GtLinkDest : public GtObject
{
public:
    enum LinkType {
        LinkNone,
        ScrollTo,
        LaunchURI
    };

public:
    explicit GtLinkDest();
    explicit GtLinkDest(const QString &uri);
    GtLinkDest(int page, const QPointF &point, double zoom);
    ~GtLinkDest();

public:
    inline LinkType type() const { return m_type; }

    inline int page() const { Q_ASSERT(m_type == ScrollTo); return m_page; }
    inline QPointF point() const { Q_ASSERT(m_type == ScrollTo); return m_point; }
    inline double zoom() const { Q_ASSERT(m_type == ScrollTo); return m_zoom; }
    void setScrollTo(int page, const QPointF &point, double zoom);

    inline QString uri() const { Q_ASSERT(m_type == LaunchURI); return m_uri; }
    void setLaunchUri(const QString &uri);

private:
    LinkType m_type;
    int m_page;
    QPointF m_point;
    double m_zoom;
    QString m_uri;
};

GT_END_NAMESPACE

#endif  /* __GT_LINK_DEST_H__ */
