/*
 * Copyright (C) 2013 Tom Wong. All rights reserved.
 */
#include "gtftserver.h"
#include "gtftsession.h"
#include <QtCore/QDebug>
#include <QtCore/QDir>

GT_BEGIN_NAMESPACE

class GtFTServerPrivate
{
    Q_DECLARE_PUBLIC(GtFTServer)

public:
    GtFTServerPrivate(GtFTServer *q);
    ~GtFTServerPrivate();

protected:
    GtFTServer *q_ptr;
};

GtFTServerPrivate::GtFTServerPrivate(GtFTServer *q)
    : q_ptr(q)
{
}

GtFTServerPrivate::~GtFTServerPrivate()
{
}

GtFTServer::GtFTServer(QObject *parent)
    : GtServer(parent)
    , d_ptr(new GtFTServerPrivate(this))
{
}

GtFTServer::~GtFTServer()
{
}

QString GtFTServer::tempPath() const
{
    return QDir::tempPath();
}

GtSession* GtFTServer::createSession()
{
    return new GtFTSession();
}

GT_END_NAMESPACE
