/*
 * Copyright (C) 2013 Tom Wong. All rights reserved.
 */
#ifndef __GT_USER_SESSION_H__
#define __GT_USER_SESSION_H__

#include "gtsession.h"

GT_BEGIN_NAMESPACE

class GtLoginRequest;
class GtUserSessionPrivate;

class GT_SVCE_EXPORT GtUserSession : public GtSession
{
    Q_OBJECT

public:
    explicit GtUserSession(QObject *parent = 0);
    ~GtUserSession();

public:
    QString name() const;

protected:
    void message(const char *data, int size);

private Q_SLOTS:
    void reloginLogout();

private:
    QScopedPointer<GtUserSessionPrivate> d_ptr;

private:
    Q_DISABLE_COPY(GtUserSession)
    Q_DECLARE_PRIVATE(GtUserSession)
};

GT_END_NAMESPACE

#endif  /* __GT_USER_SESSION_H__ */
