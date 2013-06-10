/*
 * Copyright (C) 2013 Tom Wong. All rights reserved.
 */
#ifndef __GT_DOCUMENT_H__
#define __GT_DOCUMENT_H__

#include "gtcommon.h"
#include <QtCore/QObject>
#include <QtCore/QSize>

GT_BEGIN_NAMESPACE

class GtAbstractDocument;
class GtDocPage;
class GtDocumentPrivate;

class GT_BASE_EXPORT GtDocument : public QObject
{
    Q_OBJECT

public:
    explicit GtDocument(GtAbstractDocument *ad, QObject *parent = 0);
    ~GtDocument();

public:
    bool isLoaded();
    bool isPageSizeUniform();
    QSize maxPageSize(double scale = 1.0, int rotation = 0);
    QSize minPageSize(double scale = 1.0, int rotation = 0);
    int pageCount();
    GtDocPage* page(int index);

Q_SIGNALS:
    void loaded(GtDocument * = 0);

private Q_SLOTS:
    void deviceDestroyed(QObject *object);
    void slotLoadDocument();

protected:
    friend class GtDocPage;
    friend class GtDocLoaderPrivate;
    GtDocument(GtDocumentPrivate &dd, QObject *parent);
    QScopedPointer<GtDocumentPrivate> d_ptr;

private:
    Q_DISABLE_COPY(GtDocument)
    Q_DECLARE_PRIVATE(GtDocument)
};

GT_END_NAMESPACE

#endif  /* __GT_DOCUMENT_H__ */
