/*
 * Copyright (C) 2013 Tom Wong. All rights reserved.
 */
#include "gtdocument.h"

GT_BEGIN_NAMESPACE

class GtDocumentPrivate
{
    Q_DECLARE_PUBLIC(GtDocument)

public:
    GtDocumentPrivate();

protected:
    GtDocument *q_ptr;
};

GtDocumentPrivate::GtDocumentPrivate()
{
}

GtDocument::GtDocument(QObject *parent)
    : QObject(parent)
    , d_ptr(new GtDocumentPrivate())
{
    d_ptr->q_ptr = this;
}

GtDocument::GtDocument(GtDocumentPrivate &dd, QObject *parent)
    : QObject(parent)
    , d_ptr(&dd)
{
    d_ptr->q_ptr = this;
}

GtDocument::~GtDocument()
{
}

bool GtDocument::loadFromFile(const QString &fileName)
{
    Q_UNUSED(fileName);
    return false;
}

GT_END_NAMESPACE
