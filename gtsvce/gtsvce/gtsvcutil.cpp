/*
 * Copyright (C) 2013 Tom Wong. All rights reserved.
 */
#include "gtsvcutil.h"
#include <QtCore/QDebug>
#include <QtCore/qendian.h>
#include <QtNetwork/QAbstractSocket>
#include <limits>

GT_BEGIN_NAMESPACE

bool GtSvcUtil::syncWrite(QAbstractSocket *socket, const char *buffer, int size)
{
    int bytesWrite = 0;
    int curWrite;

    while (bytesWrite < size) {
        curWrite = socket->write(buffer + bytesWrite, size - bytesWrite);
        if (curWrite < 0)
            return false;

        bytesWrite += curWrite;
        if (bytesWrite < size && !socket->waitForBytesWritten())
            return false;
    }

    return true;
}

bool GtSvcUtil::sendMessage(QAbstractSocket *socket,
                            int type,
                            const ::google::protobuf::Message *msg)
{
    int size = msg ? msg->ByteSize() : 0;

    Q_ASSERT(type <= std::numeric_limits<quint16>::max());
    Q_ASSERT(size + sizeof(quint16) <= std::numeric_limits<quint16>::max());

    QByteArray bytes(sizeof(quint32) + size, -1);
    char *data = bytes.data();

    *(quint16*)data = qToBigEndian<quint16>(static_cast<quint16>(size + sizeof(quint16)));
    *(quint16*)(data + sizeof(quint16)) = qToBigEndian<quint16>(static_cast<quint16>(type));

    if (msg && !msg->SerializeToArray(data + sizeof(quint32), size)) {
        qWarning() << "GtSvcUtil::sendMessage SerializeToArray failed";
        return false;
    }

    return syncWrite(socket, data, sizeof(quint32) + size);
}

bool GtSvcUtil::readData(QAbstractSocket *socket, char *buffer, int size)
{
    int bytesRead = 0;
    int curRead;

    while (bytesRead < size) {
        curRead = socket->read(buffer + bytesRead, size - bytesRead);
        if (curRead < 0)
            return false;

        bytesRead += curRead;
        if (bytesRead < size && !socket->waitForReadyRead())
            return false;
    }

    return true;
}

int GtSvcUtil::readMessage(QAbstractSocket *socket, char *buffer, int size)
{
    quint16 length;

    if (!readData(socket, (char*)&length, sizeof(length)))
        return -1;

    length = qFromBigEndian<quint16>(length);
    if (length > size) {
        qWarning() << "message bigger than buffer:" << size << length;
        return -1;
    }

    if (readData(socket, buffer, length))
        return length;

    return -1;
}

GT_END_NAMESPACE
