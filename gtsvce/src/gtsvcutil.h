/*
 * Copyright (C) 2013 Tom Wong. All rights reserved.
 */
#ifndef __GT_SVC_UTIL_H__
#define __GT_SVC_UTIL_H__

#include "gtcommon.h"
#include <google/protobuf/message.h>

class QAbstractSocket;

GT_BEGIN_NAMESPACE

class GT_SVCE_EXPORT GtSvcUtil
{
public:
    static void sendMessage(QAbstractSocket *socket,
                            int type,
                            const ::google::protobuf::Message *msg);
    static bool readData(QAbstractSocket *socket, char *buffer, int size);
    static int readMessage(QAbstractSocket *socket, char *buffer, int size);
};

GT_END_NAMESPACE

#endif  /* __GT_SVC_UTIL_H__ */
