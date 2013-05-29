/*
 * Copyright (C) 2013 Tom Wong. All rights reserved.
 */
#include "gtdocument_p.h"

GT_BEGIN_NAMESPACE

GtDocumentPrivate::GtDocumentPrivate()
{
}

GtDocumentPrivate::~GtDocumentPrivate()
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

GT_END_NAMESPACE
