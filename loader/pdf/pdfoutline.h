/*
 * Copyright (C) 2013 Tom Wong. All rights reserved.
 */
#ifndef __PDF_OUTLINE_H__
#define __PDF_OUTLINE_H__

#include "gtabstractdocument.h"

extern "C" {
#include "mupdf-internal.h"
}

GT_BEGIN_NAMESPACE

class PdfOutline : public GtAbstractOutline
{
public:
    PdfOutline(fz_context *c, fz_outline *o);
    ~PdfOutline();

public:
    void* iterator();
    void* nextIterator(void *it);
    void* childIterator(void *it);
    void* freeIterator(void *it);
    QString title(void *it);
    int page(void *it);

private:
    fz_context *context;
    fz_outline *outline;
};

GT_END_NAMESPACE

#endif  /* __PDF_OUTLINE_H__ */
