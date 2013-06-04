/*
 * Copyright (C) 2013 Tom Wong. All rights reserved.
 */
#ifndef __GT_DOCUMENT_P_H__
#define __GT_DOCUMENT_P_H__

#include "gtdocument.h"

GT_BEGIN_NAMESPACE

class GT_BASE_EXPORT GtDocumentPrivate
{
    Q_DECLARE_PUBLIC(GtDocument)

public:
    GtDocumentPrivate();
    virtual ~GtDocumentPrivate();

protected:
    void initialize(QObject *device);

protected:
    GtDocument *q_ptr;
    GtDocPage **pages;
    int pageCount;
    double uniformWidth;
    double uniformHeight;
    double maxWidth;
    double maxHeight;
    double minWidth;
    double minHeight;
    bool uniform;
    bool initialized;
    bool destroyed;
};

GT_END_NAMESPACE

#endif  /* __GT_DOCUMENT_P_H__ */
