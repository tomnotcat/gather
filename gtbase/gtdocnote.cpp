/*
 * Copyright (C) 2013 Tom Wong. All rights reserved.
 */
#include "gtdocnote_p.h"
#include <QtCore/QDebug>

GT_BEGIN_NAMESPACE

GtDocNotePrivate::GtDocNotePrivate(GtDocNote *q)
    : q_ptr(q)
{
}

GtDocNotePrivate::~GtDocNotePrivate()
{
}

GtDocNote::GtDocNote(QObject *parent)
    : QObject(parent)
    , d_ptr(new GtDocNotePrivate(this))
{
}

GtDocNote::GtDocNote(GtDocNotePrivate &dd, QObject* parent)
    : QObject(parent)
    , d_ptr(&dd)
{
}

GtDocNote::~GtDocNote()
{
}

GT_END_NAMESPACE
