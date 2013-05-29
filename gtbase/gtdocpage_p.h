/*
 * Copyright (C) 2013 Tom Wong. All rights reserved.
 */
#ifndef __GT_DOC_PAGE_P_H__
#define __GT_DOC_PAGE_P_H__

#include "gtdocpage.h"

GT_BEGIN_NAMESPACE

class GT_BASE_EXPORT GtDocPagePrivate
{
    Q_DECLARE_PUBLIC(GtDocPage)

public:
    GtDocPagePrivate();
    virtual ~GtDocPagePrivate();

protected:
    GtDocPage *q_ptr;
};

GT_END_NAMESPACE

#endif  /* __GT_DOC_PAGE_P_H__ */
