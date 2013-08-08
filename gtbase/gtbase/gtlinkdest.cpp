/*
 * Copyright (C) 2013 Tom Wong. All rights reserved.
 */
#include "gtlinkdest.h"
#include "gtdocmessage.pb.h"
#include "gtserialize.h"
#include <QtCore/QDebug>

GT_BEGIN_NAMESPACE

GtLinkDest::GtLinkDest()
    : m_type(LinkNone)
    , m_page(-1)
    , m_zoom(0)
{
}

GtLinkDest::GtLinkDest(const QString &uri)
    : m_type(LaunchURI)
    , m_page(-1)
    , m_zoom(0)
    , m_uri(uri)
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

void GtLinkDest::setScrollTo(int page, const QPointF &point, double zoom)
{
    m_type = ScrollTo;
    m_page = page;
    m_point = point;
    m_zoom = zoom;
}

void GtLinkDest::setLaunchUri(const QString &uri)
{
    m_type = LaunchURI;
    m_uri = uri;
}

void GtLinkDest::serialize(GtLinkDestMsg &msg) const
{
    switch (m_type) {
    case GtLinkDest::ScrollTo:
        msg.set_type(m_type);
        msg.set_page(m_page);
        msg.set_x(m_point.x());
        msg.set_y(m_point.y());
        msg.set_zoom(m_zoom);
        break;

    case GtLinkDest::LaunchURI:
        msg.set_type(m_type);
        msg.set_uri(m_uri.toUtf8());
        break;

    default:
        break;
    }
}

bool GtLinkDest::deserialize(const GtLinkDestMsg &msg)
{
    switch (msg.type()) {
    case GtLinkDest::ScrollTo:
        setScrollTo(msg.page(), QPointF(msg.x(), msg.y()), msg.zoom());
        return true;

    case GtLinkDest::LaunchURI:
        setLaunchUri(msg.uri().c_str());
        return true;

    default:
        break;
    }

    return false;
}

#ifndef QT_NO_DATASTREAM

QDataStream &operator<<(QDataStream &s, const GtLinkDest &l)
{
    return GtSerialize::serialize<GtLinkDest, GtLinkDestMsg>(s, l);
}

QDataStream &operator>>(QDataStream &s, GtLinkDest &l)
{
    return GtSerialize::deserialize<GtLinkDest, GtLinkDestMsg>(s, l);
}

#endif // QT_NO_DATASTREAM

GT_END_NAMESPACE
