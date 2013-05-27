/*
 * Copyright (C) 2013 Tom Wong. All rights reserved.
 */
#ifndef __GT_DOC_VIEW_H__
#define __GT_DOC_VIEW_H__

#include "gtcommon.h"
#include <QtWidgets/qwidget.h>

GT_BEGIN_NAMESPACE

class GtDocViewPrivate;

class GtDocView : public QWidget
{
    Q_OBJECT

public:
    explicit GtDocView(QWidget *parent = 0, Qt::WindowFlags f = 0);
    ~GtDocView();

protected:
    GtDocView(GtDocViewPrivate &dd, QWidget *parent = 0, Qt::WindowFlags f = 0);
    QScopedPointer<GtDocViewPrivate> d_ptr;

private:
    Q_DISABLE_COPY(GtDocView)
    Q_DECLARE_PRIVATE(GtDocView)
};

GT_END_NAMESPACE

#endif  /* __GT_DOC_VIEW_H__ */
