/*
 * Copyright (C) 2013 Tom Wong. All rights reserved.
 */
#ifndef __GT_SESSION_H__
#define __GT_SESSION_H__

#include "gtcommon.h"
#include <QtCore/QObject>

GT_BEGIN_NAMESPACE

class GtSessionPrivate;

class GT_SVCE_EXPORT GtSession : public QObject
{
    Q_OBJECT

public:
    explicit GtSession(QObject *parent = 0);
    ~GtSession();

private:
    QScopedPointer<GtSessionPrivate> d_ptr;

private:
    Q_DISABLE_COPY(GtSession)
    Q_DECLARE_PRIVATE(GtSession)
};

GT_END_NAMESPACE

#endif  /* __GT_SESSION_H__ */
