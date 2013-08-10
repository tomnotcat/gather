/*
 * Copyright (C) 2013 Tom Wong. All rights reserved.
 */
#ifndef __GT_USER_MANAGER_P_H__
#define __GT_USER_MANAGER_P_H__

#include "gtusermanager.h"

GT_BEGIN_NAMESPACE

class GtUserManagerProxy : public QObject, public GtObject
{
    Q_OBJECT

public:
    GtUserManagerProxy();
    ~GtUserManagerProxy();
};

GT_END_NAMESPACE

#endif  /* __GT_USER_MANAGER_P_H__ */
