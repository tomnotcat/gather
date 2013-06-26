/*
 * Copyright (C) 2013 Tom Wong. All rights reserved.
 */
#include "gtsession.h"
#include <QtCore/QDebug>

GT_BEGIN_NAMESPACE

class GtSessionPrivate
{
    Q_DECLARE_PUBLIC(GtSession)

public:
    GtSessionPrivate();
    ~GtSessionPrivate();

protected:
    GtSession *q_ptr;
};

GtSessionPrivate::GtSessionPrivate()
{
}

GtSessionPrivate::~GtSessionPrivate()
{
}

GtSession::GtSession(QObject *parent)
    : QObject(parent)
    , d_ptr(new GtSessionPrivate())
{
    d_ptr->q_ptr = this;
}

GtSession::~GtSession()
{
}

GT_END_NAMESPACE
