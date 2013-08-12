/*
 * Copyright (C) 2013 Tom Wong. All rights reserved.
 */
#ifndef __GT_DOC_MANAGER_H__
#define __GT_DOC_MANAGER_H__

#include "gtobject.h"
#include <QtCore/QObject>

class QUndoStack;

GT_BEGIN_NAMESPACE

class GtBookmark;
class GtBookmarks;
class GtDocMeta;
class GtDocModel;
class GtDocNote;
class GtDocNotes;
class GtDocument;
class GtDocManagerPrivate;

class GT_APP_EXPORT GtDocManager : public QObject, public GtObject
{
    Q_OBJECT;

public:
    explicit GtDocManager(const QString &docdb = QString(),
                          QThread *thread = 0,
                          QObject *parent = 0);
    ~GtDocManager();

public:
    int registerLoaders(const QString &loaderDir);
    int documentCount() const;
    int cleanUnreferencedObjects();
    GtDocMeta *loadDocMeta(const QString &fileId);
    GtBookmarks *loadBookmarks(const QString &bookmarksId);
    GtDocNotes *loadDocNotes(const QString &notesId);
    GtDocModel *loadDocument(const QString &fileId);
    GtDocModel *loadLocalDocument(const QString &fileName);
    QUndoStack *undoStack(GtDocModel *docModel);

private Q_SLOTS:
    void bookmarkAdded(GtBookmark *bookmark);
    void bookmarkRemoved(GtBookmark *bookmark);
    void bookmarkUpdated(GtBookmark *bookmark, int flags);
    void noteAdded(GtDocNote *note);
    void noteRemoved(GtDocNote *note);
    void updateDatabase();

private:
    QScopedPointer<GtDocManagerPrivate> d_ptr;

private:
    Q_DISABLE_COPY(GtDocManager)
    Q_DECLARE_PRIVATE(GtDocManager)
};

GT_END_NAMESPACE

#endif  /* __GT_DOC_MANAGER_H__ */
