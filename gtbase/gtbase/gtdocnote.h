/*
 * Copyright (C) 2013 Tom Wong. All rights reserved.
 */
#ifndef __GT_DOC_NOTE_H__
#define __GT_DOC_NOTE_H__

#include "gtdocrange.h"

GT_BEGIN_NAMESPACE

class GtDocNoteMsg;

class GT_BASE_EXPORT GtDocNote : public GtObject
{
public:
    enum NoteType {
        InvalidNote,
        Highlight,
        Underline
    };

public:
    explicit inline GtDocNote() : m_type(InvalidNote) {}
    GtDocNote(NoteType type, const GtDocRange &range);
    ~GtDocNote();

public:
    inline NoteType type() const { return m_type; }

    inline GtDocRange range() const { return m_range; }
    void setRange(const GtDocRange &range);

    bool isValid() const;

    void serialize(GtDocNoteMsg &msg) const;
    bool deserialize(const GtDocNoteMsg &msg);

private:
    NoteType m_type;
    GtDocRange m_range;

private:
    Q_DISABLE_COPY(GtDocNote)
};

#ifndef QT_NO_DATASTREAM
GT_BASE_EXPORT QDataStream &operator<<(QDataStream &, const GtDocNote &);
GT_BASE_EXPORT QDataStream &operator>>(QDataStream &, GtDocNote &);
#endif

GT_END_NAMESPACE

#endif  /* __GT_DOC_NOTE_H__ */
