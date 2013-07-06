/*
 * Copyright (C) 2013 Tom Wong. All rights reserved.
 */
#ifndef __GT_FT_SERVER_H__
#define __GT_FT_SERVER_H__

#include "gtserver.h"

GT_BEGIN_NAMESPACE

class GtFTSession;
class GtFTServerPrivate;

class GT_SVCE_EXPORT GtFTServer : public GtServer
{
    Q_OBJECT

public:
    explicit GtFTServer(QObject *parent = 0);
    ~GtFTServer();

protected:
    GtSession* createSession();

private:
    QScopedPointer<GtFTServerPrivate> d_ptr;

private:
    Q_DISABLE_COPY(GtFTServer)
    Q_DECLARE_PRIVATE(GtFTServer)
};

GT_END_NAMESPACE

#endif  /* __GT_FT_SERVER_H__ */
