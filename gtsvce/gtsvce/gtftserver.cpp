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
    QString tempPath;
};

GtFTServerPrivate::GtFTServerPrivate(GtFTServer *q)
    : q_ptr(q)
    , tempPath(QDir::tempPath())
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

void GtFTServer::setTempPath(const QString &path)
{
    Q_D(GtFTServer);
    d->tempPath = path;
}

QString GtFTServer::tempPath() const
{
    Q_D(const GtFTServer);
    return d->tempPath;
}

GtSession* GtFTServer::createSession()
{
    return new GtFTSession();
}

GT_END_NAMESPACE
