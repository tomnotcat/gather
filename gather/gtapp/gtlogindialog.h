/*
 * Copyright (C) 2013 Tom Wong. All rights reserved.
 */
#ifndef __GT_LOGIN_DIALOG_H__
#define __GT_LOGIN_DIALOG_H__

#include "gttabview.h"
#include "gtusermanager.h"
#include "ui_gtlogindialog.h"

GT_BEGIN_NAMESPACE

class GT_APP_EXPORT GtLoginDialog : public QDialog, public GtObject
{
    Q_OBJECT

public:
    explicit GtLoginDialog(QWidget *parent = 0);
    ~GtLoginDialog();

private Q_SLOTS:
    void on_rememberMe_clicked(bool checked);
    void on_autoLogin_clicked(bool checked);
    void on_loginButton_clicked();
    void accountIndexChanged(int index);
    void usernameEdited(const QString &text);
    void passwordEdited(const QString &text);

private:
    int indexFromUsername(const QString &username);

private:
    Ui_LoginDialog m_ui;
    QList<GtUserManager::Account> m_accounts;
    QString m_password;
};

GT_END_NAMESPACE

#endif  /* __GT_LOGIN_DIALOG_H__ */
