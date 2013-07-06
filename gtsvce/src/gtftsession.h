/*
 * Copyright (C) 2013 Tom Wong. All rights reserved.
 */
#ifndef __GT_FT_SESSION_H__
#define __GT_FT_SESSION_H__

#include "gtsession.h"

GT_BEGIN_NAMESPACE

class GtFTSessionPrivate;

class GT_SVCE_EXPORT GtFTSession : public GtSession
{
    Q_OBJECT

public:
    enum AuthResult {
        AuthSuccess,
        InvalidSession,
        InvalidSignature,
        AuthUnknown = -1
    };

public:
    explicit GtFTSession(QObject *parent = 0);
    ~GtFTSession();

protected:
    void message(const char *data, int size);

private:
    QScopedPointer<GtFTSessionPrivate> d_ptr;

private:
    Q_DISABLE_COPY(GtFTSession)
    Q_DECLARE_PRIVATE(GtFTSession)
};

GT_END_NAMESPACE

#endif  /* __GT_FT_SESSION_H__ */
