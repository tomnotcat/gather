/*
 * Copyright (C) 2013 Tom Wong. All rights reserved.
 */
#ifndef __GT_RECV_BUFFER_H__
#define __GT_RECV_BUFFER_H__

#include "gtobject.h"
#include <QtCore/QtGlobal>

class QAbstractSocket;

GT_BEGIN_NAMESPACE

class GT_SVCE_EXPORT GtRecvBuffer : public GtObject
{
public:
    explicit GtRecvBuffer();
    ~GtRecvBuffer();

public:
    inline void setMaxSize(quint16 size) { _maxSize = size; }
    inline quint16 maxSize() const { return _maxSize; }
    inline const char* buffer() const { return _buffer; }
    inline quint16 size() const { return _received - sizeof(_remain); }
    inline void clear() { _remain = 0; _received = 0; }

public:
    enum {
        ReadMessage = 1,
        ReadError = -1
    };

    int read(QAbstractSocket *socket);

private:
    char *_buffer;
    quint16 _bufferSize;
    quint16 _maxSize;
    quint16 _remain;
    quint16 _received;

private:
    Q_DISABLE_COPY(GtRecvBuffer)
};

GT_END_NAMESPACE

#endif  /* __GT_RECV_BUFFER_H__ */
