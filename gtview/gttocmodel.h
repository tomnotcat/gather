/*
 * Copyright (C) 2013 Tom Wong. All rights reserved.
 */
#ifndef __GT_TOC_MODEL_H__
#define __GT_TOC_MODEL_H__

#include "gtcommon.h"
#include <QtCore/QAbstractItemModel>

GT_BEGIN_NAMESPACE

class GtDocument;
class GtTocModelPrivate;

class GT_BASE_EXPORT GtTocModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    explicit GtTocModel(QObject *parent = 0);
    ~GtTocModel();

public:
    GtDocument* document() const;
    void setDocument(GtDocument *document);

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
    void documentDestroyed(QObject *object);

private:
    QScopedPointer<GtTocModelPrivate> d_ptr;

private:
    Q_DISABLE_COPY(GtTocModel)
    Q_DECLARE_PRIVATE(GtTocModel)
};

GT_END_NAMESPACE

#endif  /* __GT_TOC_MODEL_H__ */
