/*
 * Copyright (C) 2013 Tom Wong. All rights reserved.
 */
#ifndef __GT_SVC_UTIL_H__
#define __GT_SVC_UTIL_H__

#include "gtcommon.h"
#include <QtCore/qendian.h>
#include <QtNetwork/QAbstractSocket>
#include <google/protobuf/message.h>

GT_BEGIN_NAMESPACE

class GT_SVCE_EXPORT GtSvcUtil
{
public:
    static bool syncWrite(QAbstractSocket *socket, const char *buffer, int size);
    static bool sendMessage(QAbstractSocket *socket,
                            int type,
                            const ::google::protobuf::Message *msg);
    static bool readData(QAbstractSocket *socket, char *buffer, int size);
    static int readMessage(QAbstractSocket *socket, char *buffer, int size);

    template<typename T>
    static bool syncRequest(QAbstractSocket *socket,
                            int requestType,
                            const ::google::protobuf::Message *request,
                            int responseType,
                            T *response);
};

template<typename T>
bool GtSvcUtil::syncRequest(QAbstractSocket *socket,
                            int requestType,
                            const ::google::protobuf::Message *request,
                            int responseType,
                            T *response)
{
    if (!GtSvcUtil::sendMessage(socket, requestType, request))
        return false;

    if (!socket->waitForBytesWritten())
        return false;

    char buffer[1024];
    int length;

    length = GtSvcUtil::readMessage(socket, buffer, sizeof(buffer));
    if (length < (int)sizeof(quint16))
        return false;

    if (qFromBigEndian<quint16>(*(quint16*)buffer) != responseType)
        return false;

    if (!response->ParseFromArray(buffer + 2, length - 2))
        return false;

    return true;
}

GT_END_NAMESPACE

#endif  /* __GT_SVC_UTIL_H__ */
