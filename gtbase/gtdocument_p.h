/*
 * Copyright (C) 2013 Tom Wong. All rights reserved.
 */
#ifndef __GT_DOCUMENT_P_H__
#define __GT_DOCUMENT_P_H__

#include "gtdocument.h"
#include <QtCore/QMutex>
#include <QtCore/QSharedDataPointer>

class QIODevice;

GT_BEGIN_NAMESPACE

class GtAbstractPage;
class GtAbstractOutline;
class GtDocText;

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
    void cacheText(int index, const QSharedDataPointer<GtDocText> &text);
    inline QMutex* mutex() { return &_mutex; }

protected:
    void loadOutline(GtAbstractOutline *outline,
                     GtDocOutline *parent,
                     GtDocOutline **node, void *it);

protected:
    GtDocument *q_ptr;
    QIODevice *device;
    GtDocPage **pages;
    GtDocOutline *outline;
    int pageCount;
    double maxWidth;
    double maxHeight;
    double minWidth;
    double minHeight;
    bool uniform;
    bool loaded;
    bool destroyed;
    QMutex _mutex;
    QList<int> cachedPage;
    QList<int> cachedText;
    QScopedPointer<GtAbstractDocument> abstractDoc;
};

GT_END_NAMESPACE

#endif  /* __GT_DOCUMENT_P_H__ */
