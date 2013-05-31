/*
 * Copyright (C) 2013 Tom Wong. All rights reserved.
 */
#include "pdfdocument.h"
#include "gtdocument_p.h"

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

bool PdfDocument::load(QDataStream &stream)
{
    Q_UNUSED(stream);
    return false;
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

GT_END_NAMESPACE
