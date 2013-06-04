/*
 * Copyright (C) 2013 Tom Wong. All rights reserved.
 */
#ifndef __GT_DOCUMENT_H__
#define __GT_DOCUMENT_H__

#include "gtcommon.h"
#include <QtCore/qobject.h>

GT_BEGIN_NAMESPACE

class GtDocPage;
class GtDocumentPrivate;

class GT_BASE_EXPORT GtDocument : public QObject
{
    Q_OBJECT

public:
    explicit GtDocument(QObject *parent = 0);
    ~GtDocument();

public:
    virtual int countPages() = 0;
    virtual GtDocPage* loadPage(int page) = 0;

public:
    bool uniformPageSize(double *width, double *height);
    void maxPageSize(double *width, double *height);
    void minPageSize(double *width, double *height);
    int pageCount();
    GtDocPage* page(int index);

protected:
    friend class GtDocLoaderPrivate;
    GtDocument(GtDocumentPrivate &dd, QObject *parent);
    QScopedPointer<GtDocumentPrivate> d_ptr;

private:
    Q_DISABLE_COPY(GtDocument)
    Q_DECLARE_PRIVATE(GtDocument)
};

GT_END_NAMESPACE

#endif  /* __GT_DOCUMENT_H__ */
