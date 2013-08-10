/*
 * Copyright (C) 2013 Tom Wong. All rights reserved.
 */
#ifndef __GT_DOC_LOADER_P_H__
#define __GT_DOC_LOADER_P_H__

#include "gtdocloader.h"
#include <QtCore/QMutex>

GT_BEGIN_NAMESPACE

class GtDocLoaderProxy : public QObject, public GtObject
{
    Q_OBJECT

public:
    explicit GtDocLoaderProxy();
    ~GtDocLoaderProxy();

public:
    void load(GtDocument *document);

private Q_SLOTS:
    void loadDocument();
    void documentDestroyed(QObject *object);

private:
    QMutex m_mutex;
    QList<GtDocument*> m_documents;
};

GT_END_NAMESPACE

#endif  /* __GT_DOC_LOADER_P_H__ */
