/*
 * Copyright (C) 2013 Tom Wong. All rights reserved.
 */
#include "gtserver.h"
#include "gtsessionmanager.h"
#include <QtCore/QDebug>

GT_BEGIN_NAMESPACE

GtServer::GtServer(QObject *parent)
    : QTcpServer(parent)
{
    sessionManager = new GtSessionManager(this);
}

GtServer::~GtServer()
{
}

void GtServer::incomingConnection(qintptr socketDescriptor)
{
    sessionManager->addSession(socketDescriptor);
}

GT_END_NAMESPACE
