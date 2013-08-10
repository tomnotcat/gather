/*
 * Copyright (C) 2013 Tom Wong. All rights reserved.
 */
#ifndef __GT_HOME_TAB_VIEW_H__
#define __GT_HOME_TAB_VIEW_H__

#include "gttabview.h"
#include "ui_gthometabview.h"

GT_BEGIN_NAMESPACE

class GT_APP_EXPORT GtHomeTabView : public GtTabView
{
    Q_OBJECT

public:
    explicit GtHomeTabView(QWidget *parent = 0);
    ~GtHomeTabView();

private Q_SLOTS:
    void on_loginButton_clicked();

private:
    Ui_HomeTabView ui;
};

GT_END_NAMESPACE

#endif  /* __GT_HOME_TAB_VIEW_H__ */
