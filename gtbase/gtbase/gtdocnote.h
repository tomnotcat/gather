/*
 * Copyright (C) 2013 Tom Wong. All rights reserved.
 */
#ifndef __GT_DOC_NOTE_H__
#define __GT_DOC_NOTE_H__

#include "gtdocrange.h"

GT_BEGIN_NAMESPACE

class GT_BASE_EXPORT GtDocNote : public GtObject
{
public:
    enum NoteType {
        NullNote,
        Highlight,
        Underline
    };

public:
    GtDocNote(NoteType type, const GtDocRange &range);
    ~GtDocNote();

public:
    inline NoteType type() const { return m_type; }

    inline GtDocRange range() const { return m_range; }
    void setRange(const GtDocRange &range);

    bool isValid() const;

private:
    NoteType m_type;
    GtDocRange m_range;

private:
    Q_DISABLE_COPY(GtDocNote)
};

GT_END_NAMESPACE

#endif  /* __GT_DOC_NOTE_H__ */
