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
    QString title();
    int countPages();
    GtAbstractPage* loadPage(int index);
    GtAbstractOutline* loadOutline();

protected:
    void parseLabels(pdf_obj *tree);
    QString indexToLabel(int index);

protected:
    static int readPdfStream(fz_stream *stm, unsigned char *buf, int len);
    static void seekPdfStream(fz_stream *stm, int offset, int whence);
    static void closePdfStream(fz_context *ctx, void *state);
    static QString toRoman(int number, bool uppercase);
    static QString toLatin(int number, bool uppercase);

protected:
    class LabelRange;

private:
    fz_context *_context;
    fz_document *document;
    QList<LabelRange*> labelRanges;
};

GT_END_NAMESPACE

#endif  /* __PDF_DOCUMENT_H__ */
