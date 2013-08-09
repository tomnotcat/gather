/*
 * Copyright (C) 2013 Tom Wong. All rights reserved.
 */
#include "gttocview.h"
#include "gtbookmarks.h"
#include "gtdoccommand.h"
#include "gtdocmodel.h"
#include "gttocdelegate.h"
#include "gttocmodel.h"
#include <QtCore/QDebug>
#include <QtGui/QMouseEvent>

GT_BEGIN_NAMESPACE

class GtTocViewPrivate
{
    Q_DECLARE_PUBLIC(GtTocView)

public:
    explicit GtTocViewPrivate(GtTocView *q);
    ~GtTocViewPrivate();

private:
    GtTocView *q_ptr;
    GtTocModel *m_model;
    QUndoStack *m_undoStack;
};

GtTocViewPrivate::GtTocViewPrivate(GtTocView *q)
    : q_ptr(q)
    , m_model(0)
    , m_undoStack(0)
{
}

GtTocViewPrivate::~GtTocViewPrivate()
{
}

GtTocView::GtTocView(QWidget *parent)
    : QTreeView(parent)
    , d_ptr(new GtTocViewPrivate(this))
{
    setFrameShape(QFrame::NoFrame);
    setHeaderHidden(true);
    setUniformRowHeights(true);
    setItemDelegate(new GtTocDelegate(this));
    setEditTriggers(QAbstractItemView::EditKeyPressed);
    setContextMenuPolicy(Qt::CustomContextMenu);

    // drag and drop
    setAcceptDrops(true);
    setDragEnabled(true);
    // FIXME: auto expand seems not work properly
    setAutoExpandDelay(1000);
    setDragDropMode(QAbstractItemView::InternalMove);

    // signals
    connect(this, SIGNAL(clicked(QModelIndex)),
            this, SLOT(gotoBookmark(QModelIndex)));

    connect(this, SIGNAL(activated(QModelIndex)),
            this, SLOT(gotoBookmark(QModelIndex)));
}

GtTocView::~GtTocView()
{
}

GtTocModel *GtTocView::model() const
{
    Q_D(const GtTocView);
    return d->m_model;
}

void GtTocView::setModel(GtTocModel *model)
{
    Q_D(GtTocView);

    if (model == d->m_model)
        return;

    d->m_model = model;

    QTreeView::setModel(model);
}

QUndoStack *GtTocView::undoStack() const
{
    Q_D(const GtTocView);
    return d->m_undoStack;
}

void GtTocView::setUndoStack(QUndoStack *undoStack)
{
    Q_D(GtTocView);

    if (undoStack == d->m_undoStack)
        return;

    if (d->m_undoStack) {
        disconnect(d->m_undoStack,
                   SIGNAL(destroyed(QObject*)),
                   this,
                   SLOT(undoStackDestroyed(QObject*)));

        if (d->m_undoStack->parent() == this)
            delete d->m_undoStack;
    }

    d->m_undoStack = undoStack;
    if (d->m_undoStack) {
        connect(d->m_undoStack,
                SIGNAL(destroyed(QObject*)),
                this,
                SLOT(undoStackDestroyed(QObject*)));
    }
}

GtBookmark *GtTocView::currentBookmark() const
{
    Q_D(const GtTocView);

    if (!d->m_model)
        return 0;

    return d->m_model->bookmarkFromIndex(currentIndex());
}

void GtTocView::renameBookmark()
{
    QModelIndex index = currentIndex();

    if (!index.isValid())
        return;

    edit(index);
}

void GtTocView::deleteBookmark()
{
    Q_D(GtTocView);

    QModelIndex index = currentIndex();

    if (!index.isValid())
        return;

    int row = index.row();
    QModelIndex prev;
    QModelIndex next = index.sibling(row + 1, 0);

    if (!next.isValid())
        prev = index.sibling(row - 1, 0);

    GtDocModel *docModel = d->m_model->docModel();
    GtBookmark *bookmark = d->m_model->bookmarkFromIndex(index);
    QUndoCommand *command = new GtDelBookmarkCommand(docModel, bookmark);
    d->m_undoStack->push(command);

    if (next.isValid())
        setCurrentIndex(next.sibling(row, 0));
    else if (prev.isValid())
        setCurrentIndex(prev);
    else
        setCurrentIndex(QModelIndex());
}

void GtTocView::gotoBookmark(QModelIndex index)
{
    Q_D(GtTocView);
    emit gotoBookmark(d->m_model->bookmarkFromIndex(index));
}

void GtTocView::undoStackDestroyed(QObject *object)
{
    Q_D(GtTocView);

    if (object == static_cast<QObject *>(d->m_undoStack))
        setUndoStack(0);
}

void GtTocView::mouseMoveEvent(QMouseEvent *e)
{
    Q_D(GtTocView);

    bool dragging = (state() == QAbstractItemView::DraggingState);
    if (dragging)
        d->m_model->beginDrag(indexAt(e->pos()));

    QTreeView::mouseMoveEvent(e);

    if (dragging) {
        GtBookmark *dropped = d->m_model->endDrag();
        if (dropped)
            setCurrentIndex(d->m_model->indexFromBookmark(dropped));
    }
}

GT_END_NAMESPACE
