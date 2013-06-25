/*
 * Copyright (C) 2013 Tom Wong. All rights reserved.
 */
#include "gttocmodel.h"
#include "gtdocoutline.h"
#include "gtdocpage.h"
#include "gtdocument.h"
#include "gttocdelegate.h"
#include <QtCore/QDebug>

GT_BEGIN_NAMESPACE

class GtTocModelPrivate
{
    Q_DECLARE_PUBLIC(GtTocModel)

public:
    GtTocModelPrivate();
    ~GtTocModelPrivate();

protected:
    GtTocModel *q_ptr;
    GtDocument *document;
    GtDocOutline *outline;
};

GtTocModelPrivate::GtTocModelPrivate()
    : document(0)
    , outline(0)
{
}

GtTocModelPrivate::~GtTocModelPrivate()
{
}

GtTocModel::GtTocModel(QObject *parent)
    : QAbstractItemModel(parent)
    , d_ptr(new GtTocModelPrivate())
{
    d_ptr->q_ptr = this;
}

GtTocModel::~GtTocModel()
{
}

GtDocument* GtTocModel::document() const
{
    Q_D(const GtTocModel);
    return d->document;
}

void GtTocModel::setDocument(GtDocument *document)
{
    Q_D(GtTocModel);

    if (document == d->document)
        return;

    if (d->document) {
        disconnect(d->document,
                   SIGNAL(destroyed(QObject*)),
                   this,
                   SLOT(documentDestroyed(QObject*)));
    }

    d->document = document;
    if (d->document) {
        d->outline = d->document->outline();

        connect(d->document,
                SIGNAL(destroyed(QObject*)),
                this,
                SLOT(documentDestroyed(QObject*)));
    }
}

QModelIndex GtTocModel::index(int row, int column,
                              const QModelIndex &parent) const
{
    Q_D(const GtTocModel);

    if (!hasIndex(row, column, parent))
        return QModelIndex();

    GtDocOutline *node;
    if (!parent.isValid())
        node = d->outline;
    else
        node = static_cast<GtDocOutline*>(parent.internalPointer());

    GtDocOutline *child = node->child(row);
    if (child)
        return createIndex(row, column, child);

    return QModelIndex();
}

QModelIndex GtTocModel::parent(const QModelIndex &child) const
{
    if (!child.isValid())
        return QModelIndex();

    GtDocOutline *node = static_cast<GtDocOutline*>(child.internalPointer());
    GtDocOutline *parent = node->parent();

    if (0 == parent)
        return QModelIndex();

    return createIndex(parent->row, 0, static_cast<void *>(parent));
}

int GtTocModel::rowCount(const QModelIndex &parent) const
{
    Q_D(const GtTocModel);

    if (!d->outline)
        return 0;

    if (parent.column() > 0)
        return 0;

    GtDocOutline *node;
    if (!parent.isValid())
        node = d->outline;
    else
        node = static_cast<GtDocOutline*>(parent.internalPointer());

    return node->childCount();
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

    GtDocOutline *node = static_cast<GtDocOutline*>(index.internalPointer());
    switch (role) {
    case Qt::DisplayRole:
    case Qt::ToolTipRole:
        return QVariant(node->title);

    case GtTocDelegate::PageIndex:
        return QVariant(node->page + 1);

    case GtTocDelegate::PageLabel:
        return QVariant(d->document->page(node->page)->label());

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

void GtTocModel::documentDestroyed(QObject *object)
{
    Q_D(GtTocModel);

    if (object == static_cast<QObject *>(d->document))
        setDocument(0);
}

GT_END_NAMESPACE
