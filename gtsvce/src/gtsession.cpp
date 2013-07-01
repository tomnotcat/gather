/*
 * Copyright (C) 2013 Tom Wong. All rights reserved.
 */
#include "gtsession_p.h"
#include <QtCore/QDebug>
#include <QtNetwork/QAbstractSocket>

GT_BEGIN_NAMESPACE

GtSessionPrivate::GtSessionPrivate(GtSession *q)
    : q_ptr(q)
    , socket(0)
{
}

GtSessionPrivate::~GtSessionPrivate()
{
    socket->deleteLater();
}

GtSession::GtSession(QObject *parent)
    : QObject(parent)
    , d_ptr(new GtSessionPrivate(this))
{
}

GtSession::~GtSession()
{
}

QAbstractSocket* GtSession::socket() const
{
    Q_D(const GtSession);
    return d->socket;
}

GT_END_NAMESPACE
