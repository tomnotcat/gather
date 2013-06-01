/*
 * Copyright (C) 2013 Tom Wong. All rights reserved.
 */
#include "pdfdocument.h"
#include "gtdocument_p.h"
#include <QtCore/QDebug>

GT_BEGIN_NAMESPACE

class PdfDocumentPrivate : public GtDocumentPrivate
{
    Q_DECLARE_PUBLIC(GtDocument)

public:
    PdfDocumentPrivate();
    ~PdfDocumentPrivate();
};

PdfDocumentPrivate::PdfDocumentPrivate()
{
}

PdfDocumentPrivate::~PdfDocumentPrivate()
{
}

PdfDocument::PdfDocument(QObject *parent)
    : GtDocument(*new PdfDocumentPrivate(), parent)
{
}

PdfDocument::~PdfDocument()
{
}

int PdfDocument::countPages()
{
    return 0;
}

GtDocPage* PdfDocument::loadPage(int page)
{
    Q_UNUSED(page);
    return 0;
}

GtDocument* gather_load_document(QIODevice *device)
{
    Q_UNUSED(device);
    return new PdfDocument();
}

GT_END_NAMESPACE
