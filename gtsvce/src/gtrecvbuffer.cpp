/*
 * Copyright (C) 2013 Tom Wong. All rights reserved.
 */
#include "gtrecvbuffer.h"
#include "gtsvcutil.h"
#include <QtCore/QDebug>
#include <QtNetwork/QAbstractSocket>

GT_BEGIN_NAMESPACE

GtRecvBuffer::GtRecvBuffer()
    : _buffer(0)
    , _bufferSize(0)
    , _maxSize(0)
    , _remain(0)
    , _received(0)
{
}

GtRecvBuffer::~GtRecvBuffer()
{
    delete[] _buffer;
}

int GtRecvBuffer::read(QAbstractSocket *socket)
{
    qint64 bytesRead;

    if (_received < sizeof(_remain)) {
        bytesRead = socket->read((char *)&_remain + _received,
                                 sizeof(_remain) - _received);
        if (bytesRead < 0)
            return ReadError;

        _received += bytesRead;
        if (_received < sizeof(_remain))
            return 0;

        _remain = ntohs(_remain);
        if (_bufferSize < _remain) {
            if (_maxSize > 0 && _remain > _maxSize)
                return ReadError;

            delete[] _buffer;
            _buffer = new char[_remain];
            _bufferSize = _remain;
        }
    }

    bytesRead = socket->read(_buffer + _received - sizeof(_remain), _remain);
    if (bytesRead < 0)
        return ReadError;

    _remain -= bytesRead;
    _received += bytesRead;
    if (0 == _remain)
        return ReadMessage;

    return 0;
}

GT_END_NAMESPACE
