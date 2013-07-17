/*
 * Copyright (C) 2013 Tom Wong. All rights reserved.
 */
#ifndef __GT_TOC_DELEGATE_H__
#define __GT_TOC_DELEGATE_H__

#include "gtobject.h"
#include <QtWidgets/QItemDelegate>

GT_BEGIN_NAMESPACE

class GtTocDelegatePrivate;

class GT_VIEW_EXPORT GtTocDelegate : public QItemDelegate, public GtObject
{
    Q_OBJECT

public:
    enum {
        PageLabel = 0x000f0001,
        PageIndex = 0x000f0002
    };

public:
    explicit GtTocDelegate(QObject *parent = 0);
    ~GtTocDelegate();

protected:
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;

    void drawDisplay(QPainter *painter, const QStyleOptionViewItem &option, const QRect &rect, const QString &text) const;

private:
    QScopedPointer<GtTocDelegatePrivate> d_ptr;

private:
    Q_DISABLE_COPY(GtTocDelegate)
    Q_DECLARE_PRIVATE(GtTocDelegate)
};

GT_END_NAMESPACE

#endif  /* __GT_TOC_DELEGATE_H__ */
