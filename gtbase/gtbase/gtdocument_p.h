/*
 * Copyright (C) 2013 Tom Wong. All rights reserved.
 */
#ifndef __GT_DOCUMENT_P_H__
#define __GT_DOCUMENT_P_H__

#include "gtdocument.h"
#include "gtdocpage.h"
#include <QtCore/QMutex>
#include <QtCore/QSharedDataPointer>

class QIODevice;

GT_BEGIN_NAMESPACE

class GtAbstractPage;
class GtAbstractOutline;

class GtDocumentPrivate
{
    Q_DECLARE_PUBLIC(GtDocument)

public:
    explicit GtDocumentPrivate(GtAbstractDocument *a,
                               GtDocument *q);
    virtual ~GtDocumentPrivate();

public:
    void setDevice(const QString &title, QIODevice *device);
    GtAbstractPage* lockPage(int index);
    void unlockPage(int index);
    void cacheText(int index, const GtDocTextPointer &text);
    inline QMutex* mutex() { return &m_mutex; }

protected:
    int loadOutline(GtAbstractOutline *outline,
                    GtBookmark *parent, void *it);

protected:
    GtDocument *q_ptr;
    QIODevice *m_device;
    GtDocPage **m_pages;
    QString m_fileId;
    QString m_title;
    int m_pageCount;
    double m_maxWidth;
    double m_maxHeight;
    double m_minWidth;
    double m_minHeight;
    bool m_uniform;
    bool m_loaded;
    bool m_destroyed;
    QMutex m_mutex;
    QList<int> m_cachedPage;
    QList<int> m_cachedText;
    QScopedPointer<GtAbstractDocument> m_abstractDoc;
};

GT_END_NAMESPACE

#endif  /* __GT_DOCUMENT_P_H__ */
