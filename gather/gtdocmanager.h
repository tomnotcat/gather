/*
 * Copyright (C) 2013 Tom Wong. All rights reserved.
 */
#ifndef __GT_DOC_MANAGER_H__
#define __GT_DOC_MANAGER_H__

#include "gtdocmodel.h"
#include <QtCore/QHash>

GT_BEGIN_NAMESPACE

class GtDocLoader;

class GtDocManager : public QObject, public GtObject
{
    Q_OBJECT;

public:
    explicit GtDocManager(QObject *parent = 0);
    ~GtDocManager();

public:
    int registerLoaders(const QString &loaderDir);
    inline void setDocThread(QThread *thread) { m_docThread = thread; }
    GtDocModelPointer loadDocument(const QString &fileName);

private:
    void clearDocuments();

private:
    GtDocLoader *m_docLoader;
    QThread *m_docThread;
    QHash<QString, GtDocModelPointer> m_docModels;
};

GT_END_NAMESPACE

#endif  /* __GT_DOC_MANAGER_H__ */
