/*
 * Copyright (C) 2013 Tom Wong. All rights reserved.
 */
#ifndef __GT_DOC_NOTES_H__
#define __GT_DOC_NOTES_H__

#include "gtobject.h"
#include <QtCore/QObject>

GT_BEGIN_NAMESPACE

class GtDocNote;
class GtDocNotesPrivate;

class GT_BASE_EXPORT GtDocNotes : public QObject, public GtObject
{
    Q_OBJECT

public:
    explicit GtDocNotes(QObject *parent = 0);
    ~GtDocNotes();

public:
    int pageCount() const;
    QList<GtDocNote*> pageNotes(int index) const;

protected:
    QScopedPointer<GtDocNotesPrivate> d_ptr;

private:
    Q_DISABLE_COPY(GtDocNotes)
    Q_DECLARE_PRIVATE(GtDocNotes)
};

GT_END_NAMESPACE

#endif  /* __GT_DOC_NOTES_H__ */
