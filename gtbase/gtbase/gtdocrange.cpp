/*
 * Copyright (C) 2013 Tom Wong. All rights reserved.
 */
#include "gtdocrange.h"
#include "gtdocmessage.pb.h"
#include "gtdocpage.h"
#include "gtserialize.h"
#include <QtCore/QDebug>

GT_BEGIN_NAMESPACE

QPoint GtDocRange::intersectedText(GtDocPage *page) const
{
    GtDocument *document = page->document();
    int beginText = m_begin.text(document, false);
    int endText = m_end.text(document, false);

    if (-1 == beginText || -1 == endText)
        return QPoint();

    int beginIndex = m_begin.page();
    int endIndex = m_end.page();
    int index = page->index();

    if (index < beginIndex || index > endIndex)
        return QPoint();

    if (index == beginIndex) {
        if (index == endIndex) {
            if (endText > beginText)
                return QPoint(beginText, endText);

            return QPoint(endText, beginText);
        }

        return QPoint(beginText, page->length());
    }

    if (index == endIndex)
        return QPoint(0, endText);

    return QPoint(0, page->length());
}

void GtDocRange::serialize(GtDocRangeMsg &msg) const
{
    msg.set_type(m_type);
    msg.set_begin_page(m_begin.page());
    msg.set_begin_x(m_begin.x());
    msg.set_begin_y(m_begin.y());
    msg.set_end_page(m_end.page());
    msg.set_end_x(m_end.x());
    msg.set_end_y(m_end.y());
}

bool GtDocRange::deserialize(const GtDocRangeMsg &msg)
{
    GtDocPoint begin(msg.begin_page(), msg.begin_x(), msg.begin_y());
    GtDocPoint end(msg.end_page(), msg.end_x(), msg.end_y());

    switch (msg.type()) {
    case GtDocRange::TextRange:
    case GtDocRange::GeomRange:
        setType((GtDocRange::RangeType)msg.type());
        setRange(begin, end);
        return true;

    default:
        break;
    }

    return false;
}

#ifndef QT_NO_DATASTREAM

QDataStream &operator<<(QDataStream &s, const GtDocRange &r)
{
    return GtSerialize::serialize<GtDocRange, GtDocRangeMsg>(s, r);
}

QDataStream &operator>>(QDataStream &s, GtDocRange &r)
{
    return GtSerialize::deserialize<GtDocRange, GtDocRangeMsg>(s, r);
}

#endif // QT_NO_DATASTREAM

GT_END_NAMESPACE
