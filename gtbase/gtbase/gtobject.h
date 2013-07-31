/*
 * Copyright (C) 2013 Tom Wong. All rights reserved.
 */
#ifndef __GT_OBJECT_H__
#define __GT_OBJECT_H__

#include "gtcommon.h"
#include <QtCore/qatomic.h>

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

class GT_BASE_EXPORT GtSharedObject : public GtObject
{
public:
    mutable QAtomicInt ref;

    inline GtSharedObject() : ref(0) { }
    inline GtSharedObject(const GtSharedObject &) : ref(0) { }

public:
    inline void release() { if (!ref.deref()) delete this; }

private:
    // using the assignment operator would lead to corruption in the ref-counting
    GtSharedObject &operator=(const GtSharedObject &);
};

GT_END_NAMESPACE

#endif  /* __GT_OBJECT_H__ */
