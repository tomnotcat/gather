/*
 * Copyright (C) 2013 Tom Wong. All rights reserved.
 */
#include "gttocdelegate.h"
#include <QtCore/QDebug>
#include <QtGui/QTextDocument>
#include <QtWidgets/QApplication>

GT_BEGIN_NAMESPACE

#define TOCDELEGATE_INTERNALMARGIN 3

class GtTocDelegatePrivate
{
    Q_DECLARE_PUBLIC(GtTocDelegate)

public:
    GtTocDelegatePrivate();
    ~GtTocDelegatePrivate();

protected:
    GtTocDelegate *q_ptr;
    QModelIndex index;
};

GtTocDelegatePrivate::GtTocDelegatePrivate()
{
}

GtTocDelegatePrivate::~GtTocDelegatePrivate()
{
}

GtTocDelegate::GtTocDelegate(QObject *parent)
    : QItemDelegate(parent)
    , d_ptr(new GtTocDelegatePrivate())
{
    d_ptr->q_ptr = this;
}

GtTocDelegate::~GtTocDelegate()
{
}

void GtTocDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index ) const
{
    d_ptr->index = index;
    QItemDelegate::paint(painter, option, index);
}

void GtTocDelegate::drawDisplay(QPainter *painter, const QStyleOptionViewItem &option, const QRect &rect, const QString &text) const
{
    Q_D(const GtTocDelegate);

    QVariant pageLabel = d->index.data(PageLabel);
    QVariant pageIndex = d->index.data(PageIndex);

    if ((pageLabel.type() != QVariant::String &&
         !pageIndex.canConvert(QVariant::String)))
    {
        QItemDelegate::drawDisplay(painter, option, rect, text);
        return;
    }

    QString label = pageLabel.toString();
    QString page = label.isEmpty() ? pageIndex.toString() : label;

    QTextDocument document;
    document.setPlainText(page);
    document.setDefaultFont(option.font);
    int margindelta = QApplication::style()->pixelMetric(QStyle::PM_FocusFrameHMargin) + 1;
    int pageRectWidth = (int)document.size().width();
    QRect newRect(rect);
    QRect pageRect(rect);
    pageRect.setWidth(pageRectWidth + 2 * margindelta);
    newRect.setWidth(newRect.width() - pageRectWidth - TOCDELEGATE_INTERNALMARGIN);
    if (option.direction == Qt::RightToLeft)
        newRect.translate(pageRectWidth + TOCDELEGATE_INTERNALMARGIN, 0);
    else
        pageRect.translate(newRect.width() + TOCDELEGATE_INTERNALMARGIN - 2 * margindelta, 0);

    QItemDelegate::drawDisplay(painter, option, newRect, text);
    QStyleOptionViewItemV2 newoption(option);
    newoption.displayAlignment = (option.displayAlignment & ~Qt::AlignHorizontal_Mask) | Qt::AlignRight;
    QItemDelegate::drawDisplay(painter, newoption, pageRect, page);
}

GT_END_NAMESPACE
