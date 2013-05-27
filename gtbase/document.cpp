/*
 * Copyright (C) 2013 Tom Wong. All rights reserved.
 */
#include "document.h"

GATHER_BEGIN_NAMESPACE

class DocumentPrivate
{
    Q_DECLARE_PUBLIC(Document)

public:
    DocumentPrivate();

private:
    Document *q_ptr;
};

DocumentPrivate::DocumentPrivate()
{
}

Document::Document()
{
}

Document::~Document()
{
}

bool Document::loadFromFile(const QString &fileName)
{
    Q_UNUSED(fileName);
    return false;
}

GATHER_END_NAMESPACE
