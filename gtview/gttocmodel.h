/*
 * Copyright (C) 2013 Tom Wong. All rights reserved.
 */
#ifndef __GT_TOC_MODEL_H__
#define __GT_TOC_MODEL_H__

#include "gtobject.h"
#include <QtCore/QAbstractItemModel>

class QUndoCommand;
class QUndoStack;

GT_BEGIN_NAMESPACE

class GtBookmark;
class GtBookmarks;
class GtDocModel;
class GtTocModelPrivate;

class GT_VIEW_EXPORT GtTocModel : public QAbstractItemModel, public GtObject
{
    Q_OBJECT

public:
    explicit GtTocModel(QObject *parent = 0);
    ~GtTocModel();

public:
    GtDocModel* docModel() const;
    void setDocModel(GtDocModel *docModel);

    QUndoStack* undoStack() const;
    void setUndoStack(QUndoStack *undoStack);

    void beginDrag(const QModelIndex &index);
    GtBookmark* endDrag();

    GtBookmark* bookmarkFromIndex(const QModelIndex &index) const;
    QModelIndex indexFromBookmark(GtBookmark* bookmark) const;

public:
    QModelIndex index(int row, int column,
                      const QModelIndex &parent) const;
    QModelIndex parent(const QModelIndex &child) const;
    int rowCount(const QModelIndex &parent) const;
    int columnCount(const QModelIndex &parent) const;
    QVariant data(const QModelIndex &index, int role) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role);
    Qt::ItemFlags flags(const QModelIndex &index) const;
    Qt::DropActions supportedDropActions() const;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    bool removeRows(int row, int count, const QModelIndex &parent);
    bool dropMimeData(const QMimeData *data, Qt::DropAction action,
                      int row, int column, const QModelIndex &parent);
    QMimeData *mimeData(const QModelIndexList &indexes) const;
    QStringList mimeTypes() const;

private Q_SLOTS:
    void bookmarksChanged(GtBookmarks *bookmarks);
    void bookmarkAdded(GtBookmark *bookmark);
    void bookmarkRemoved(GtBookmark *bookmark);
    void bookmarkUpdated(GtBookmark *bookmark, int flags);
    void docModelDestroyed(QObject *object);
    void undoStackDestroyed(QObject *object);

private:
    QScopedPointer<GtTocModelPrivate> d_ptr;

private:
    Q_DISABLE_COPY(GtTocModel)
    Q_DECLARE_PRIVATE(GtTocModel)
};

GT_END_NAMESPACE

#endif  /* __GT_TOC_MODEL_H__ */
