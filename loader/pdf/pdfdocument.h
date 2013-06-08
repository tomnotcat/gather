/*
 * Copyright (C) 2013 Tom Wong. All rights reserved.
 */
#ifndef __PDF_DOCUMENT_H__
#define __PDF_DOCUMENT_H__

#include "gtdocument.h"
#include <QtCore/qplugin.h>

extern "C" {
#include "mupdf-internal.h"
}

class QIODevice;

GT_BEGIN_NAMESPACE

class PdfDocumentPrivate;

class PdfDocument : public GtDocument
{
    Q_OBJECT

public:
    explicit PdfDocument(QObject *parent = 0);
    ~PdfDocument();

protected:
    bool loadDocument();
    int countPages();
    GtDocPage* loadPage(int index);

private:
    Q_DISABLE_COPY(PdfDocument)
    Q_DECLARE_PRIVATE(PdfDocument)
};

Q_EXTERN_C GtDocument* gather_new_document(void);

GT_END_NAMESPACE

#endif  /* __PDF_DOCUMENT_H__ */
