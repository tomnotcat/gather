/*
 * Copyright (C) 2013 Tom Wong. All rights reserved.
 */
#ifndef __GT_OBJECT_H__
#define __GT_OBJECT_H__

#include "gtcommon.h"

GT_BEGIN_NAMESPACE

class GT_BASE_EXPORT GtObject
{
#ifdef GT_DEBUG
public:
    explicit GtObject();
    virtual ~GtObject();

public:
    static int dumpObjects();
#endif  /* !GT_DEBUG */
};

GT_END_NAMESPACE

#endif  /* __GT_OBJECT_H__ */
