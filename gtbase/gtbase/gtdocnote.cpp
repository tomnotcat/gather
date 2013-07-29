/*
 * Copyright (C) 2013 Tom Wong. All rights reserved.
 */
#include "gtdocnote.h"
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

GT_END_NAMESPACE
