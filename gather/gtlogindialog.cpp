/*
 * Copyright (C) 2013 Tom Wong. All rights reserved.
 */
#include "gtlogindialog.h"
#include <QtCore/QDebug>

GT_BEGIN_NAMESPACE

GtLoginDialog::GtLoginDialog(QWidget *parent)
    : QDialog(parent)
{
    ui.setupUi(this);
    ui.accountEdit->lineEdit()->setPlaceholderText(tr("please enter account"));

    connect(ui.cancelButton, SIGNAL(clicked()), this, SLOT(close()));
}

GtLoginDialog::~GtLoginDialog()
{
}

void GtLoginDialog::on_loginButton_clicked()
{
    qDebug() << ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>";
}

GT_END_NAMESPACE
