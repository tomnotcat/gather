/*
 * Copyright (C) 2013 Tom Wong. All rights reserved.
 */
#include "gtnotemanager.h"
#include <QtCore/QDebug>

GT_BEGIN_NAMESPACE

class GtNoteManagerPrivate
{
    Q_DECLARE_PUBLIC(GtNoteManager)

public:
    GtNoteManagerPrivate(GtNoteManager *q);
    ~GtNoteManagerPrivate();

protected:
    GtNoteManager *q_ptr;
};

GtNoteManagerPrivate::GtNoteManagerPrivate(GtNoteManager *q)
    : q_ptr(q)
{
}

GtNoteManagerPrivate::~GtNoteManagerPrivate()
{
}

GtNoteManager::GtNoteManager(QObject *parent)
    : QObject(parent)
    , d_ptr(new GtNoteManagerPrivate(this))
{
}

GtNoteManager::~GtNoteManager()
{
}

GT_END_NAMESPACE
