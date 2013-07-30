/*
 * Copyright (C) 2013 Tom Wong. All rights reserved.
 */
#include "gttocmodel.h"
#include "gtbookmark.h"
#include "gtbookmarks.h"
#include "gtdocmodel.h"
#include "gtdocpage.h"
#include "gtdocument.h"
#include "gttocdelegate.h"
#include <QtCore/QDebug>

GT_BEGIN_NAMESPACE

class GtTocModelPrivate
{
    Q_DECLARE_PUBLIC(GtTocModel)

public:
    GtTocModelPrivate(GtTocModel *q);
    ~GtTocModelPrivate();

protected:
    GtTocModel *q_ptr;
    GtDocModel *m_docModel;
    GtBookmarks *m_bookmarks;
};

GtTocModelPrivate::GtTocModelPrivate(GtTocModel *q)
    : q_ptr(q)
    , m_docModel(0)
    , m_bookmarks(0)
{
}

GtTocModelPrivate::~GtTocModelPrivate()
{
}

GtTocModel::GtTocModel(QObject *parent)
    : QAbstractItemModel(parent)
    , d_ptr(new GtTocModelPrivate(this))
{
}

GtTocModel::~GtTocModel()
{
}

GtDocModel* GtTocModel::docModel() const
{
    Q_D(const GtTocModel);
    return d->m_docModel;
}

void GtTocModel::setDocModel(GtDocModel *docModel)
{
    Q_D(GtTocModel);

    if (docModel == d->m_docModel)
        return;

    if (d->m_docModel) {
        disconnect(d->m_docModel,
                   SIGNAL(bookmarksChanged(GtBookmarks*)),
                   this,
                   SLOT(bookmarksChanged(GtBookmarks*)));

        disconnect(d->m_docModel,
                   SIGNAL(destroyed(QObject*)),
                   this,
                   SLOT(docModelDestroyed(QObject*)));
    }

    if (d->m_bookmarks) {
        disconnect(d->m_bookmarks,
                   SIGNAL(inserted(GtBookmark*)),
                   this,
                   SLOT(bookmarkInserted(GtBookmark*)));
    }

    d->m_docModel = docModel;
    d->m_bookmarks = 0;

    if (d->m_docModel) {
        d->m_bookmarks = d->m_docModel->bookmarks();

        connect(d->m_docModel,
                SIGNAL(bookmarksChanged(GtBookmarks*)),
                this,
                SLOT(bookmarksChanged(GtBookmarks*)));

        connect(d->m_docModel,
                SIGNAL(destroyed(QObject*)),
                this,
                SLOT(docModelDestroyed(QObject*)));
    }

    if (d->m_bookmarks) {
        connect(d->m_bookmarks,
                SIGNAL(inserted(GtBookmark*)),
                this,
                SLOT(bookmarkInserted(GtBookmark*)));
    }
}

GtBookmark* GtTocModel::bookmarkFromIndex(const QModelIndex &index) const
{
    if (!index.isValid())
        return 0;

    return static_cast<GtBookmark*>(index.internalPointer());
}

QModelIndex GtTocModel::indexFromBookmark(GtBookmark* bookmark) const
{
    if (!bookmark)
        return QModelIndex();

    return createIndex(bookmark->index(), 0, static_cast<void *>(bookmark));
}

QModelIndex GtTocModel::index(int row, int column,
                              const QModelIndex &parent) const
{
    Q_D(const GtTocModel);

    if (!hasIndex(row, column, parent))
        return QModelIndex();

    GtBookmark *node;
    if (!parent.isValid())
        node = d->m_bookmarks->root();
    else
        node = static_cast<GtBookmark*>(parent.internalPointer());

    GtBookmark *child = node->children()[row];
    if (child)
        return createIndex(row, column, child);

    return QModelIndex();
}

QModelIndex GtTocModel::parent(const QModelIndex &child) const
{
    if (!child.isValid())
        return QModelIndex();

    GtBookmark *node = static_cast<GtBookmark*>(child.internalPointer());
    GtBookmark *parent = node->parent();

    if (0 == parent)
        return QModelIndex();

    return createIndex(parent->index(), 0, static_cast<void *>(parent));
}

int GtTocModel::rowCount(const QModelIndex &parent) const
{
    Q_D(const GtTocModel);

    if (!d->m_bookmarks)
        return 0;

    if (parent.column() > 0)
        return 0;

    GtBookmark *node;
    if (!parent.isValid())
        node = d->m_bookmarks->root();
    else
        node = static_cast<GtBookmark*>(parent.internalPointer());

    return node->children().size();
}

int GtTocModel::columnCount(const QModelIndex &) const
{
    return 1;
}

QVariant GtTocModel::data(const QModelIndex &index, int role) const
{
    Q_D(const GtTocModel);

    if (!index.isValid())
        return QVariant();

    GtBookmark *node = static_cast<GtBookmark*>(index.internalPointer());
    switch (role) {
    case Qt::DisplayRole:
    case Qt::ToolTipRole:
        {
            QString title = node->title();

            if (title.isEmpty())
                title = tr("Untitled");

            return QVariant(title);
        }
        break;

    case GtTocDelegate::PageIndex:
        return QVariant(node->dest().page() + 1);

    case GtTocDelegate::PageLabel:
        {
            GtDocument *document = d->m_docModel->document();
            return QVariant(document->page(node->dest().page())->label());
        }
        break;

    default:
        break;
    }

    return QVariant();
}

Qt::ItemFlags GtTocModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return 0;

    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

QVariant GtTocModel::headerData(int, Qt::Orientation, int) const
{
    return QVariant();
}

void GtTocModel::bookmarksChanged(GtBookmarks *bookmarks)
{
    Q_D(GtTocModel);

    beginResetModel();

    if (d->m_bookmarks) {
        disconnect(d->m_bookmarks,
                   SIGNAL(inserted(GtBookmark*)),
                   this,
                   SLOT(bookmarkInserted(GtBookmark*)));
    }

    d->m_bookmarks = bookmarks;

    if (d->m_bookmarks) {
        connect(d->m_bookmarks,
                SIGNAL(inserted(GtBookmark*)),
                this,
                SLOT(bookmarkInserted(GtBookmark*)));
    }

    endResetModel();
}

void GtTocModel::bookmarkInserted(GtBookmark *)
{
    emit layoutChanged();
}

void GtTocModel::docModelDestroyed(QObject *object)
{
    Q_D(GtTocModel);

    if (object == static_cast<QObject *>(d->m_docModel))
        setDocModel(0);
}

GT_END_NAMESPACE
