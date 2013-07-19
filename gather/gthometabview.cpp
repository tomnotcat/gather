/*
 * Copyright (C) 2013 Tom Wong. All rights reserved.
 */
#include "gthometabview.h"
#include "gtlogindialog.h"
#include "gtmainwindow.h"

GT_BEGIN_NAMESPACE

GtHomeTabView::GtHomeTabView(QWidget *parent)
    : GtTabView(parent)
{
    ui.setupUi(this);
}

GtHomeTabView::~GtHomeTabView()
{
}

void GtHomeTabView::on_loginButton_clicked()
{
    GtLoginDialog dialog(this);
    dialog.exec();
}

GT_END_NAMESPACE
