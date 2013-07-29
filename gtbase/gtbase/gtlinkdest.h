/*
 * Copyright (C) 2013 Tom Wong. All rights reserved.
 */
#ifndef __GT_LINK_DEST_H__
#define __GT_LINK_DEST_H__

#include "gtobject.h"
#include <QtCore/QPointF>

GT_BEGIN_NAMESPACE

class GT_BASE_EXPORT GtLinkDest : public GtObject
{
public:
    enum LinkType {
        LinkNone,
        LinkGoto
    };

public:
    explicit GtLinkDest();
    GtLinkDest(int page, const QPointF &point, double zoom);
    ~GtLinkDest();

public:
    inline LinkType type() const { return m_type; }
    inline int page() const { return m_data.gotor.page; }
    inline QPointF point() const {
        return QPointF(m_data.gotor.x, m_data.gotor.y); }
    inline double zoom() const { return m_data.gotor.zoom; }

private:
    LinkType m_type;
    union {
        struct {
            int page;
            double x;
            double y;
            double zoom;
        }
        gotor;
    } m_data;
};

GT_END_NAMESPACE

#endif  /* __GT_LINK_DEST_H__ */
