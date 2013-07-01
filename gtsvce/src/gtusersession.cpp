/*
 * Copyright (C) 2013 Tom Wong. All rights reserved.
 */
#include "gtusersession.h"
#include <QtCore/QDebug>

GT_BEGIN_NAMESPACE

class GtUserSessionPrivate
{
    Q_DECLARE_PUBLIC(GtUserSession)

public:
    GtUserSessionPrivate(GtUserSession *q);
    ~GtUserSessionPrivate();

protected:
    GtUserSession *q_ptr;
};

GtUserSessionPrivate::GtUserSessionPrivate(GtUserSession *q)
    : q_ptr(q)
{
}

GtUserSessionPrivate::~GtUserSessionPrivate()
{
}

GtUserSession::GtUserSession(QObject *parent)
    : GtSession(parent)
    , d_ptr(new GtUserSessionPrivate(this))
{
}

GtUserSession::~GtUserSession()
{
}

GT_END_NAMESPACE
