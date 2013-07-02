/*
 * Copyright (C) 2013 Tom Wong. All rights reserved.
 */
#include <QApplication>
#include "gtmainwindow.h"

using namespace Gather;

int main(int argc, char *argv[])
{
    int result = 0;

    if (1) {
        QApplication app(argc, argv);
        GtMainWindow mainWin;
        mainWin.show();
        result = app.exec();
    }

#ifdef GT_DEBUG
    Q_ASSERT(GtObject::dumpObjects() == 0);
#endif
    return result;
}
