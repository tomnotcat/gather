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
    GtDocModel *docModel;
    GtBookmark *bookmark;
};

GtTocModelPrivate::GtTocModelPrivate(GtTocModel *q)
    : q_ptr(q)
    , docModel(0)
    , bookmark(0)
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
    return d->docModel;
}

void GtTocModel::setDocModel(GtDocModel *docModel)
{
    Q_D(GtTocModel);

    if (docModel == d->docModel)
        return;

    if (d->docModel) {
        disconnect(d->docModel,
                   SIGNAL(destroyed(QObject*)),
                   this,
                   SLOT(docModelDestroyed(QObject*)));
    }

    d->docModel = docModel;
    d->bookmark = 0;

    if (d->docModel) {
        d->bookmark = docModel->bookmarks()->root();

        connect(d->docModel,
                SIGNAL(destroyed(QObject*)),
                this,
                SLOT(docModelDestroyed(QObject*)));
    }
}

GtBookmark* GtTocModel::bookmarkFromIndex(const QModelIndex &index) const
{
    if (!index.isValid())
        return 0;

    return static_cast<GtBookmark*>(index.internalPointer());
}

QModelIndex GtTocModel::index(int row, int column,
                              const QModelIndex &parent) const
{
    Q_D(const GtTocModel);

    if (!hasIndex(row, column, parent))
        return QModelIndex();

    GtBookmark *node;
    if (!parent.isValid())
        node = d->bookmark;
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

    if (!d->bookmark)
        return 0;

    if (parent.column() > 0)
        return 0;

    GtBookmark *node;
    if (!parent.isValid())
        node = d->bookmark;
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
        return QVariant(node->title());

    case GtTocDelegate::PageIndex:
        return QVariant(node->dest().page() + 1);

    case GtTocDelegate::PageLabel:
        {
            GtDocument *document = d->docModel->document();
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

void GtTocModel::docModelDestroyed(QObject *object)
{
    Q_D(GtTocModel);

    if (object == static_cast<QObject *>(d->docModel))
        setDocModel(0);
}

GT_END_NAMESPACE
