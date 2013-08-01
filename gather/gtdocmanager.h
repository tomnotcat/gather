/*
 * Copyright (C) 2013 Tom Wong. All rights reserved.
 */
#ifndef __GT_DOC_MANAGER_H__
#define __GT_DOC_MANAGER_H__

#include "gtobject.h"
#include <QtCore/QObject>

class QUndoStack;

GT_BEGIN_NAMESPACE

class GtBookmarks;
class GtDocMeta;
class GtDocModel;
class GtDocNotes;
class GtDocument;
class GtDocManagerPrivate;

class GtDocManager : public QObject, public GtObject
{
    Q_OBJECT;

public:
    explicit GtDocManager(QThread *thread = 0, QObject *parent = 0);
    ~GtDocManager();

public:
    int registerLoaders(const QString &loaderDir);
    GtDocMeta *loadDocMeta(const QString &fileId);
    GtDocModel *loadDocument(const QString &fileId);
    GtDocModel *loadLocalDocument(const QString &fileName);
    GtBookmarks *loadBookmarks(const QString &bookmarksId);
    GtDocNotes *loadNotes(const QString &notesId);
    QUndoStack *undoStack(GtDocModel *docModel);

private Q_SLOTS:
    void documentLoaded(GtDocument *document);
    void docMetaChanged(GtDocMeta *meta);

private:
    QScopedPointer<GtDocManagerPrivate> d_ptr;

private:
    Q_DISABLE_COPY(GtDocManager)
    Q_DECLARE_PRIVATE(GtDocManager)
};

GT_END_NAMESPACE

#endif  /* __GT_DOC_MANAGER_H__ */
