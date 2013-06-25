/*
 * Copyright (C) 2013 Tom Wong. All rights reserved.
 */
#include "gttocmodel.h"
#include "gtdocoutline.h"
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
    GtDocOutline *outline;
};

GtTocModelPrivate::GtTocModelPrivate()
    : outline(0)
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

GtDocOutline* GtTocModel::outline() const
{
    Q_D(const GtTocModel);
    return d->outline;
}

void GtTocModel::setOutline(GtDocOutline *outline)
{
    Q_D(GtTocModel);

    if (outline == d->outline)
        return;

    if (d->outline) {
        disconnect(d->outline,
                   SIGNAL(destroyed(QObject*)),
                   this,
                   SLOT(outlineDestroyed(QObject*)));
    }

    d->outline = outline;
    if (d->outline) {
        connect(d->outline,
                SIGNAL(destroyed(QObject*)),
                this,
                SLOT(outlineDestroyed(QObject*)));
    }
}

QModelIndex GtTocModel::index(int row, int column,
                              const QModelIndex &parent) const
{
    Q_D(const GtTocModel);

    if (!hasIndex(row, column, parent))
        return QModelIndex();

    GtDocOutline::Node *node;
    if (!parent.isValid())
        node = d->outline->first();
    else
        node = static_cast<GtDocOutline::Node*>(parent.internalPointer());

    GtDocOutline::Node *child = node->child(row);
    if (child)
        return createIndex(row, column, child);

    return QModelIndex();
}

QModelIndex GtTocModel::parent(const QModelIndex &child) const
{
    if (!child.isValid())
        return QModelIndex();

    GtDocOutline::Node *node = static_cast<GtDocOutline::Node*>(child.internalPointer());
    GtDocOutline::Node *parent = node->parent();

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

    GtDocOutline::Node *node;
    if (!parent.isValid())
        node = d->outline->first();
    else
        node = static_cast<GtDocOutline::Node*>(parent.internalPointer());

    return node->childCount();
}

int GtTocModel::columnCount(const QModelIndex &parent) const
{
    if(parent.isValid())
        return 1;

    return 0;
}

QVariant GtTocModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (role != Qt::DisplayRole)
        return QVariant();

    GtDocOutline::Node *node = static_cast<GtDocOutline::Node*>(index.internalPointer());

    return QVariant(node->title);
}

void GtTocModel::outlineDestroyed(QObject *object)
{
    Q_D(GtTocModel);

    if (object == static_cast<QObject *>(d->outline))
        setOutline(0);
}

GT_END_NAMESPACE
