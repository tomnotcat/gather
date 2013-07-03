/*
 * Copyright (C) 2013 Tom Wong. All rights reserved.
 */
#ifndef __GT_USER_SERVER_H__
#define __GT_USER_SERVER_H__

#include "gtserver.h"

GT_BEGIN_NAMESPACE

class GtUserSession;
class GtUserServerPrivate;

class GT_SVCE_EXPORT GtUserServer : public GtServer
{
    Q_OBJECT

public:
    explicit GtUserServer(QObject *parent = 0);
    ~GtUserServer();

public:
    void addLogin(GtUserSession *session);

protected:
    GtSession* createSession();
    void removeSession(GtSession *session);

private:
    QScopedPointer<GtUserServerPrivate> d_ptr;

private:
    Q_DISABLE_COPY(GtUserServer)
    Q_DECLARE_PRIVATE(GtUserServer)
};

GT_END_NAMESPACE

#endif  /* __GT_USER_SERVER_H__ */
