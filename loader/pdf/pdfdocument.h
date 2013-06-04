/*
 * Copyright (C) 2013 Tom Wong. All rights reserved.
 */
#ifndef __PDF_DOCUMENT_H__
#define __PDF_DOCUMENT_H__

#include "gtdocument.h"
#include <QtCore/qplugin.h>

class QIODevice;

GT_BEGIN_NAMESPACE

class PdfDocumentPrivate;

class PdfDocument : public GtDocument
{
    Q_OBJECT

public:
    explicit PdfDocument(QObject *parent = 0);
    ~PdfDocument();

public:
    bool loadDocument(QIODevice *device);

public:
    int countPages();
    GtDocPage* loadPage(int page);

private:
    Q_DISABLE_COPY(PdfDocument)
    Q_DECLARE_PRIVATE(PdfDocument)
};

Q_EXTERN_C GtDocument* gather_load_document(QIODevice *device);

GT_END_NAMESPACE

#endif  /* __PDF_DOCUMENT_H__ */
