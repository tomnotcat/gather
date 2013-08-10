/*
 * Copyright (C) 2013 Tom Wong. All rights reserved.
 */
#ifndef __GT_DOCUMENT_H__
#define __GT_DOCUMENT_H__

#include "gtobject.h"
#include <QtCore/QObject>
#include <QtCore/QSize>

class QIODevice;

GT_BEGIN_NAMESPACE

class GtAbstractDocument;
class GtBookmark;
class GtDocPage;
class GtDocumentPrivate;

class GT_BASE_EXPORT GtDocument : public QObject, public GtSharedObject
{
    Q_OBJECT

public:
    explicit GtDocument(GtAbstractDocument *a, QObject *parent = 0);
    ~GtDocument();

public:
    QString fileId() const;
    QString title() const;
    bool isLoaded() const;
    bool isPageSizeUniform() const;
    QSize maxPageSize(double scale = 1.0, int rotation = 0) const;
    QSize minPageSize(double scale = 1.0, int rotation = 0) const;
    int pageCount() const;
    GtDocPage* page(int index) const;
    int loadOutline(GtBookmark *root);

public:
    static QString makeFileId(QIODevice *device);

Q_SIGNALS:
    void loaded(GtDocument * = 0);

private Q_SLOTS:
    void deviceDestroyed(QObject *object);
    void loadDocument();

protected:
    friend class GtDocPage;
    friend class GtDocLoaderPrivate;
    QScopedPointer<GtDocumentPrivate> d_ptr;

private:
    Q_DISABLE_COPY(GtDocument)
    Q_DECLARE_PRIVATE(GtDocument)
};

GT_END_NAMESPACE

#endif  /* __GT_DOCUMENT_H__ */
