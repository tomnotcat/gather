/*
 * Copyright (C) 2013 Tom Wong. All rights reserved.
 */
#include "gtlogindialog.h"
#include "gtapplication.h"
#include <QtCore/QDebug>
#include <QtWidgets/QCompleter>

GT_BEGIN_NAMESPACE

GtLoginDialog::GtLoginDialog(QWidget *parent)
    : QDialog(parent)
{
    m_ui.setupUi(this);

    // auto completion
    QLineEdit *usernameEdit = m_ui.usernameEdit->lineEdit();
    usernameEdit->setMaxLength(255);

    connect(usernameEdit, SIGNAL(textEdited(const QString &)),
            this, SLOT(usernameEdited(const QString &)));

    connect(m_ui.passwordEdit, SIGNAL(textEdited(const QString &)),
            this, SLOT(passwordEdited(const QString &)));

    QCompleter *completer = new QCompleter(m_ui.usernameEdit->model(), this);
    completer->setCaseSensitivity(Qt::CaseSensitive);
    completer->setCompletionMode(QCompleter::InlineCompletion);
    completer->setCompletionColumn(m_ui.usernameEdit->modelColumn());
    m_ui.usernameEdit->setCompleter(completer);

    // load accounts
    GtUserManager *userManager = GtApplication::instance()->userManager();
    QList<GtUserManager::Account>::const_iterator it;

    m_accounts = userManager->accounts();
    if (m_accounts.size() > 0) {
        for (int i = 0; i < m_accounts.size(); ++i) {
            m_ui.usernameEdit->addItem(m_accounts[i].username, i);
        }

        connect(m_ui.usernameEdit, SIGNAL(currentIndexChanged(int)),
                this, SLOT(accountIndexChanged(int)));

        accountIndexChanged(0);
    }
}

GtLoginDialog::~GtLoginDialog()
{
}

void GtLoginDialog::on_rememberMe_clicked(bool checked)
{
    if (!checked) {
        m_ui.autoLogin->setChecked(false);
    }

    int index = indexFromUsername(m_ui.usernameEdit->currentText());
    if (index != -1) {
        m_accounts[index].rememberMe = checked;
        m_accounts[index].autoLogin = m_ui.autoLogin->isChecked();
    }
}

void GtLoginDialog::on_autoLogin_clicked(bool checked)
{
    if (checked) {
        m_ui.rememberMe->setChecked(true);
    }

    int index = indexFromUsername(m_ui.usernameEdit->currentText());
    if (index != -1) {
        m_accounts[index].rememberMe = m_ui.rememberMe->isChecked();
        m_accounts[index].autoLogin = checked;
    }
}

void GtLoginDialog::on_loginButton_clicked()
{
    QLineEdit *usernameEdit = m_ui.usernameEdit->lineEdit();
    GtUserManager::Account account;

    account.username = usernameEdit->text();
    if (account.username.isEmpty()) {
        return;
    }

    account.password = m_password;
    if (account.password.isEmpty()) {
        return;
    }

    account.rememberMe = m_ui.rememberMe->isChecked();
    account.autoLogin = m_ui.autoLogin->isChecked();

    GtUserManager *userManager = GtApplication::instance()->userManager();
    userManager->login(account);
}

void GtLoginDialog::accountIndexChanged(int index)
{
    const GtUserManager::Account &a = m_accounts[index];

    if (a.password.isEmpty()) {
        m_ui.passwordEdit->setText(QString());
        m_ui.rememberMe->setChecked(false);
        m_ui.autoLogin->setChecked(false);
    }
    else {
        // just for display purperse
        m_ui.passwordEdit->setText("000000");
        m_ui.rememberMe->setChecked(true);
        m_ui.autoLogin->setChecked(a.autoLogin);
    }

    m_password = a.password;
}

void GtLoginDialog::usernameEdited(const QString &text)
{
    int index = indexFromUsername(text);
    if (index != -1) {
        if (m_ui.usernameEdit->currentIndex() != index) {
            m_ui.usernameEdit->setCurrentIndex(index);
        }
        else {
            accountIndexChanged(index);
        }
        return;
    }

    m_ui.rememberMe->setChecked(false);
    m_ui.autoLogin->setChecked(false);
    m_ui.passwordEdit->setText("");
    m_password.clear();
}

void GtLoginDialog::passwordEdited(const QString &text)
{
    Q_UNUSED(text);
    m_password = text;
}

int GtLoginDialog::indexFromUsername(const QString &username)
{
    for (int i = 0; i < m_accounts.size(); ++i) {
        const GtUserManager::Account &a = m_accounts[i];
        if (a.username == username)
            return i;
    }

    return -1;
}

GT_END_NAMESPACE
