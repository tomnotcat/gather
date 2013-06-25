/*
 * Copyright (C) 2013 Tom Wong. All rights reserved.
 */
#ifndef __GT_ABSTRACT_DOCUMENT_H__
#define __GT_ABSTRACT_DOCUMENT_H__

#include "gtcommon.h"

class QIODevice;
class QPaintDevice;
class QChar;
class QRectF;

GT_BEGIN_NAMESPACE

class GT_BASE_EXPORT GtAbstractPage
{
public:
    GtAbstractPage();
    virtual ~GtAbstractPage();

public:
    virtual QString label() = 0;
    virtual void size(double *width, double *height) = 0;
    virtual int textLength() = 0;
    virtual int extractText(QChar *texts, QRectF *rects, int length) = 0;
    virtual void paint(QPaintDevice *device, double scale, int rotation) = 0;
};

class GT_BASE_EXPORT GtAbstractOutline
{
public:
    GtAbstractOutline();
    virtual ~GtAbstractOutline();

public:
    virtual void* firstNode() = 0;
    virtual void* nextNode(void *node) = 0;
    virtual void* childNode(void *node);
    virtual void freeNode(void *node);
    virtual QString title(void *node) = 0;
    virtual int page(void *it) = 0;
};

class GT_BASE_EXPORT GtAbstractDocument
{
public:
    GtAbstractDocument();
    virtual ~GtAbstractDocument();

public:
    virtual bool load(QIODevice *device) = 0;
    virtual int countPages() = 0;
    virtual GtAbstractPage* loadPage(int index) = 0;
    virtual GtAbstractOutline* loadOutline() = 0;
};

#define GT_DEFINE_DOCUMENT_LOADER(constructor) \
GT_EXTERN_C Q_DECL_EXPORT int gather_module_version() { \
    return 1; \
} \
GT_EXTERN_C Q_DECL_EXPORT GtAbstractDocument* gather_new_document() { \
    return new constructor; \
}

GT_END_NAMESPACE

#endif  /* __GT_ABSTRACT_DOCUMENT_H__ */
