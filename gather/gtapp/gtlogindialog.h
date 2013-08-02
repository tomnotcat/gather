/*
 * Copyright (C) 2013 Tom Wong. All rights reserved.
 */
#ifndef __GT_LOGIN_DIALOG_H__
#define __GT_LOGIN_DIALOG_H__

#include "gttabview.h"
#include "ui_gtlogindialog.h"

GT_BEGIN_NAMESPACE

class GtLoginDialog : public QDialog, public GtObject
{
    Q_OBJECT

public:
    explicit GtLoginDialog(QWidget *parent = 0);
    ~GtLoginDialog();

private Q_SLOTS:
    void on_loginButton_clicked();

private:
    Ui_LoginDialog ui;
};

GT_END_NAMESPACE

#endif  /* __GT_LOGIN_DIALOG_H__ */
