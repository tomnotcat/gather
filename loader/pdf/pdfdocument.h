/*
 * Copyright (C) 2013 Tom Wong. All rights reserved.
 */
#ifndef __PDF_DOCUMENT_H__
#define __PDF_DOCUMENT_H__

#include "gtabstractdocument.h"
#include <QtCore/qplugin.h>

extern "C" {
#include "mupdf-internal.h"
}

GT_BEGIN_NAMESPACE

class PdfDocument : public GtAbstractDocument
{
public:
    explicit PdfDocument();
    ~PdfDocument();

public:
    bool load(QIODevice *device);
    int countPages();
    GtAbstractPage* loadPage(int index);

public:
    static fz_context* context();

protected:
    static void lockContext(void *user, int lock);
    static void unlockContext(void *user, int lock);
    static int readPdfStream(fz_stream *stm, unsigned char *buf, int len);
    static void seekPdfStream(fz_stream *stm, int offset, int whence);
    static void closePdfStream(fz_context *ctx, void *state);

private:
    fz_document *document;
};

GT_END_NAMESPACE

#endif  /* __PDF_DOCUMENT_H__ */
