/*
 * Copyright (C) 2013 Tom Wong. All rights reserved.
 */
#ifndef __GT_DOCUMENT_P_H__
#define __GT_DOCUMENT_P_H__

#include "gtdocument.h"
#include <QtCore/QMutex>

class QIODevice;

GT_BEGIN_NAMESPACE

class GtAbstractPage;

class GT_BASE_EXPORT GtDocumentPrivate
{
    Q_DECLARE_PUBLIC(GtDocument)

public:
    explicit GtDocumentPrivate(GtAbstractDocument *ad);
    virtual ~GtDocumentPrivate();

public:
    void setDevice(QIODevice *device);
    GtAbstractPage* lockPage(int index);
    void unlockPage(int index);

protected:
    GtDocument *q_ptr;
    QIODevice *device;
    GtDocPage **pages;
    int pageCount;
    int pageCacheSize;
    double maxWidth;
    double maxHeight;
    double minWidth;
    double minHeight;
    bool uniform;
    bool loaded;
    bool destroyed;
    QMutex mutex;
    QList<int> cachedPages;
    QScopedPointer<GtAbstractDocument> abstractDoc;
};

GT_END_NAMESPACE

#endif  /* __GT_DOCUMENT_P_H__ */
