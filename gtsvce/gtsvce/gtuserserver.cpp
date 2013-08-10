/*
 * Copyright (C) 2013 Tom Wong. All rights reserved.
 */
#include "gtuserserver.h"
#include "gtusersession.h"
#include <QtCore/QDebug>

GT_BEGIN_NAMESPACE

class GtUserServerPrivate
{
    Q_DECLARE_PUBLIC(GtUserServer)

public:
    GtUserServerPrivate(GtUserServer *q);
    ~GtUserServerPrivate();

protected:
    GtUserServer *q_ptr;
};

GtUserServerPrivate::GtUserServerPrivate(GtUserServer *q)
    : q_ptr(q)
{
}

GtUserServerPrivate::~GtUserServerPrivate()
{
}

GtUserServer::GtUserServer(QObject *parent)
    : GtServer(parent)
    , d_ptr(new GtUserServerPrivate(this))
{
}

GtUserServer::~GtUserServer()
{
}

GtSession* GtUserServer::createSession()
{
    return new GtUserSession();
}

void GtUserServer::removeSession(GtSession *session)
{
    Q_UNUSED(session);
}

GT_END_NAMESPACE
