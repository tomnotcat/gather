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
    inline void setMaxSize(quint16 size) { m_maxSize = size; }
    inline quint16 maxSize() const { return m_maxSize; }
    inline const char* buffer() const { return m_buffer; }
    inline quint16 size() const { return m_received - sizeof(m_remain); }
    inline void clear() { m_remain = 0; m_received = 0; }

public:
    enum {
        ReadMessage = 1,
        ReadError = -1
    };

    int read(QAbstractSocket *socket, bool wait);

private:
    char *m_buffer;
    quint16 m_bufferSize;
    quint16 m_maxSize;
    quint16 m_remain;
    quint16 m_received;

private:
    Q_DISABLE_COPY(GtRecvBuffer)
};

GT_END_NAMESPACE

#endif  /* __GT_RECV_BUFFER_H__ */
