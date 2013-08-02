/*
 * Copyright (C) 2013 Tom Wong. All rights reserved.
 */
#include "gtapplication.h"

using namespace Gather;

int main(int argc, char *argv[])
{
    int result = 0;

    if (1) {
        GtApplication application(argc, argv);
        if (!application.isTheOnlyReader())
            return 0;

        result = application.exec();
    }

#ifdef GT_DEBUG
    Q_ASSERT(GtObject::dumpObjects() == 0);
#endif
    return result;
}
