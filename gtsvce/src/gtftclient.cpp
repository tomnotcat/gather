/*
 * Copyright (C) 2013 Tom Wong. All rights reserved.
 */
#include "gtftclient.h"
#include <QtCore/QDebug>
#include <QtNetwork/QTcpSocket>

GT_BEGIN_NAMESPACE

class GtFTClientPrivate
{
    Q_DECLARE_PUBLIC(GtFTClient)

public:
    explicit GtFTClientPrivate(GtFTClient *q);
    ~GtFTClientPrivate();

protected:
    GtFTClient *q_ptr;
};

GtFTClientPrivate::GtFTClientPrivate(GtFTClient *q)
    : q_ptr(q)
{
}

GtFTClientPrivate::~GtFTClientPrivate()
{
}

GtFTClient::GtFTClient(QObject *parent)
    : QObject(parent)
    , d_ptr(new GtFTClientPrivate(this))
{
}

GtFTClient::~GtFTClient()
{
}

GT_END_NAMESPACE
