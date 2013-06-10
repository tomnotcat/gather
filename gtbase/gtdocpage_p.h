/*
 * Copyright (C) 2013 Tom Wong. All rights reserved.
 */
#ifndef __GT_DOC_PAGE_P_H__
#define __GT_DOC_PAGE_P_H__

#include "gtdocpage.h"

GT_BEGIN_NAMESPACE

class GtDocument;
class GtAbstractPage;

class GT_BASE_EXPORT GtDocPagePrivate
{
    Q_DECLARE_PUBLIC(GtDocPage)

public:
    GtDocPagePrivate();
    virtual ~GtDocPagePrivate();

public:
    inline void initialize(GtDocument *d, int i, double w, double h) {
        document = d;
        index = i;
        width = w;
        height = h;
    }

public:
    GtAbstractPage *abstractPage;

protected:
    GtDocPage *q_ptr;
    GtDocument *document;
    int index;
    int textLength;
    double width;
    double height;
};

GT_END_NAMESPACE

#endif  /* __GT_DOC_PAGE_P_H__ */
