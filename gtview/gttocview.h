/*
 * Copyright (C) 2013 Tom Wong. All rights reserved.
 */
#ifndef __GT_TOC_VIEW_H__
#define __GT_TOC_VIEW_H__

#include "gtobject.h"
#include <QtWidgets/QTreeView>

class QUndoStack;

GT_BEGIN_NAMESPACE

class GtBookmark;
class GtTocModel;
class GtTocViewPrivate;

class GT_VIEW_EXPORT GtTocView : public QTreeView, public GtObject
{
    Q_OBJECT

public:
    explicit GtTocView(QWidget *parent = 0);
    ~GtTocView();

public:
    GtTocModel *model() const;
    void setModel(GtTocModel *model);

    QUndoStack *undoStack() const;
    void setUndoStack(QUndoStack *undoStack);

    GtBookmark *currentBookmark() const;

Q_SIGNALS:
    void gotoBookmark(GtBookmark *);

public Q_SLOTS:
    void renameBookmark();
    void deleteBookmark();

private Q_SLOTS:
    void gotoBookmark(QModelIndex);
    void undoStackDestroyed(QObject *object);

protected:
    void mouseMoveEvent(QMouseEvent *);

private:
    QScopedPointer<GtTocViewPrivate> d_ptr;

private:
    Q_DISABLE_COPY(GtTocView)
    Q_DECLARE_PRIVATE(GtTocView)
};

GT_END_NAMESPACE

#endif  /* __GT_TOC_VIEW_H__ */
