/*
 * Copyright (C) 2013 Tom Wong. All rights reserved.
 */
#include "gtsvcutil.h"
#include <QtCore/QDebug>
#include <QtNetwork/QAbstractSocket>
#include <limits>

GT_BEGIN_NAMESPACE

void GtSvcUtil::sendMessage(QAbstractSocket *socket,
                            int type,
                            const ::google::protobuf::Message &msg)
{
    int size = msg.ByteSize();
    Q_ASSERT(type <= std::numeric_limits<quint16>::max());
    Q_ASSERT(size + sizeof(quint16) <= std::numeric_limits<quint16>::max());

    QByteArray bytes(sizeof(quint32) + size, -1);
    char *data = bytes.data();

    *(quint16*)data = htons(static_cast<quint16>(size + sizeof(quint16)));
    *(quint16*)(data + sizeof(quint16)) = htons(static_cast<quint16>(type));

    if (msg.SerializeToArray(data + sizeof(quint32), size)) {
        socket->write(bytes);
    }
    else {
        qWarning() << "SerializeToArray failed";
    }
}

GT_END_NAMESPACE
