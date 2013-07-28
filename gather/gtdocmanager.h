/*
 * Copyright (C) 2013 Tom Wong. All rights reserved.
 */
#ifndef __GT_DOC_MANAGER_H__
#define __GT_DOC_MANAGER_H__

#include "gtdocmodel.h"
#include <QtCore/QHash>

class QUndoStack;

GT_BEGIN_NAMESPACE

class GtDocLoader;

class GtDocManager : public QObject, public GtObject
{
    Q_OBJECT;

public:
    explicit GtDocManager(QThread *thread = 0, QObject *parent = 0);
    ~GtDocManager();

public:
    int registerLoaders(const QString &loaderDir);
    GtDocModelPointer loadDocument(const QString &fileName);
    QUndoStack* undoStack(GtDocModel *docModel);

private Q_SLOTS:
    void documentLoaded(GtDocument *document);

private:
    void clearDocuments();

private:
    GtDocLoader *m_docLoader;
    QThread *m_docThread;
    QHash<GtDocument*, GtDocModel*> m_loadingDocs;
    QHash<GtDocModel*, QUndoStack*> m_undoStatcks;
    QHash<QString, GtDocModelPointer> m_docModels;
};

GT_END_NAMESPACE

#endif  /* __GT_DOC_MANAGER_H__ */
