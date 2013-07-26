/*
 * Copyright (C) 2013 Tom Wong. All rights reserved.
 */
#ifndef __GT_TOC_MODEL_H__
#define __GT_TOC_MODEL_H__

#include "gtobject.h"
#include <QtCore/QAbstractItemModel>

GT_BEGIN_NAMESPACE

class GtBookmark;
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
    GtBookmark* bookmarkFromIndex(const QModelIndex &index) const;

public:
    QModelIndex index(int row, int column,
                      const QModelIndex &parent) const;
    QModelIndex parent(const QModelIndex &child) const;
    int rowCount(const QModelIndex &parent) const;
    int columnCount(const QModelIndex &parent) const;
    QVariant data(const QModelIndex &index, int role) const;
    Qt::ItemFlags flags(const QModelIndex &index) const;
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const;

private Q_SLOTS:
    void docModelDestroyed(QObject *object);

private:
    QScopedPointer<GtTocModelPrivate> d_ptr;

private:
    Q_DISABLE_COPY(GtTocModel)
    Q_DECLARE_PRIVATE(GtTocModel)
};

GT_END_NAMESPACE

#endif  /* __GT_TOC_MODEL_H__ */
