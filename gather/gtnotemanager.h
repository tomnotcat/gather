/*
 * Copyright (C) 2013 Tom Wong. All rights reserved.
 */
#ifndef __GT_NOTE_MANAGER_H__
#define __GT_NOTE_MANAGER_H__

#include "gtobject.h"
#include <QtCore/QObject>

GT_BEGIN_NAMESPACE

class GtNoteManagerPrivate;

class GtNoteManager : public QObject, public GtObject
{
    Q_OBJECT;

public:
    explicit GtNoteManager(QObject *parent = 0);
    ~GtNoteManager();

private:
    QScopedPointer<GtNoteManagerPrivate> d_ptr;

private:
    Q_DISABLE_COPY(GtNoteManager)
    Q_DECLARE_PRIVATE(GtNoteManager)
};

GT_END_NAMESPACE

#endif  /* __GT_NOTE_MANAGER_H__ */
