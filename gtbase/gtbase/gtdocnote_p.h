/*
 * Copyright (C) 2013 Tom Wong. All rights reserved.
 */
#ifndef __GT_DOC_NOTE_P_H__
#define __GT_DOC_NOTE_P_H__

#include "gtdocnote.h"
#include "gtdocrange.h"

GT_BEGIN_NAMESPACE

class GtDocNotePrivate
{
    Q_DECLARE_PUBLIC(GtDocNote)

public:
    explicit GtDocNotePrivate(GtDocNote *q);
    virtual ~GtDocNotePrivate();

protected:
    GtDocNote *q_ptr;
    GtDocRange range;
    QString text;
};

GT_END_NAMESPACE

#endif  /* __GT_DOC_NOTE_P_H__ */
