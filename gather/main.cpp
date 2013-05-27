/*
 * Copyright (C) 2013 Tom Wong. All rights reserved.
 */
#include <QApplication>
#include "mainwindow.h"

using namespace Gather;

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    MainWindow mainWin;
    mainWin.show();
    return app.exec();
}
