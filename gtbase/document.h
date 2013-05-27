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
    explicit Document(QObject *parent = 0);
    ~Document();

public:
    bool loadFromFile(const QString &fileName);

protected:
    const QScopedPointer<DocumentPrivate> d_ptr;
    Document(DocumentPrivate &d, QObject *parent);

private:
    Q_DISABLE_COPY(Document)
};

GATHER_END_NAMESPACE

#endif  /* GATHER_DOCUMENT_H */
