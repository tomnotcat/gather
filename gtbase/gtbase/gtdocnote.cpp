/*
 * Copyright (C) 2013 Tom Wong. All rights reserved.
 */
#include "gtdocnote.h"
#include "gtdocmessage.pb.h"
#include "gtserialize.h"
#include <QtCore/QDebug>

GT_BEGIN_NAMESPACE

GtDocNote::GtDocNote(NoteType type, const GtDocRange &range)
    : m_type(type)
    , m_range(range)
{
    Q_ASSERT(isValid());
}

GtDocNote::~GtDocNote()
{
}

void GtDocNote::setRange(const GtDocRange &range)
{
    m_range = range;
    Q_ASSERT(isValid());
}

bool GtDocNote::isValid() const
{
    if (m_range.type() == GtDocRange::UnknownRange)
        return false;

    if (Underline == m_type)
        return (m_range.type() == GtDocRange::TextRange);

    return true;
}

void GtDocNote::serialize(GtDocNoteMsg &msg) const
{
    msg.set_type(m_type);
    m_range.serialize(*msg.mutable_range());
}

bool GtDocNote::deserialize(const GtDocNoteMsg &msg)
{
    switch (msg.type()) {
    case GtDocNote::Highlight:
    case GtDocNote::Underline:
        m_type = (GtDocNote::NoteType)msg.type();
        return m_range.deserialize(msg.range());

    default:
        break;
    }

    return false;
}

#ifndef QT_NO_DATASTREAM

QDataStream &operator<<(QDataStream &s, const GtDocNote &n)
{
    return GtSerialize::serialize<GtDocNote, GtDocNoteMsg>(s, n);
}

QDataStream &operator>>(QDataStream &s, GtDocNote &n)
{
    return GtSerialize::deserialize<GtDocNote, GtDocNoteMsg>(s, n);
}

#endif // QT_NO_DATASTREAM

GT_END_NAMESPACE
