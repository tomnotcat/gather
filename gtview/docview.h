/*
 * Copyright (C) 2013 Tom Wong. All rights reserved.
 */
#ifndef GATHER_DOCUMENT_H
#define GATHER_DOCUMENT_H

#include "common.h"
#include <QtCore/qobject.h>

GATHER_BEGIN_NAMESPACE

class DocumentPrivate;

class Document : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(Document)

public:
    Document();
    ~Document();

public:
    bool loadFromFile(const QString &fileName);

private:
    Q_DISABLE_COPY(Document)
    DocumentPrivate *d_ptr;
};

GATHER_END_NAMESPACE

#endif  /* GATHER_DOCUMENT_H */
