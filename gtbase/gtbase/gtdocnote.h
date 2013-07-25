/*
 * Copyright (C) 2013 Tom Wong. All rights reserved.
 */
#ifndef __GT_DOC_NOTE_H__
#define __GT_DOC_NOTE_H__

#include "gtobject.h"
#include <QtCore/QObject>

GT_BEGIN_NAMESPACE

class GtDocRange;
class GtDocNotePrivate;

class GT_BASE_EXPORT GtDocNote : public QObject, public GtObject
{
    Q_OBJECT

public:
    explicit GtDocNote(QObject *parent = 0);
    explicit GtDocNote(const GtDocRange &range, QObject *parent = 0);
    ~GtDocNote();

public:
    GtDocRange range() const;
    void setRange(const GtDocRange &range);

    QString text() const;
    void setText(const QString &text);

protected:
    GtDocNote(GtDocNotePrivate &dd, QObject* parent);

protected:
    QScopedPointer<GtDocNotePrivate> d_ptr;

private:
    Q_DISABLE_COPY(GtDocNote)
    Q_DECLARE_PRIVATE(GtDocNote)
};

GT_END_NAMESPACE

#endif  /* __GT_DOC_NOTE_H__ */
