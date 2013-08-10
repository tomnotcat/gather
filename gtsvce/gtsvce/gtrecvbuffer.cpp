/*
 * Copyright (C) 2013 Tom Wong. All rights reserved.
 */
#include "gtrecvbuffer.h"
#include "gtsvcutil.h"
#include <QtCore/QDebug>
#include <QtCore/qendian.h>
#include <QtNetwork/QAbstractSocket>

GT_BEGIN_NAMESPACE

GtRecvBuffer::GtRecvBuffer()
    : m_buffer(0)
    , m_bufferSize(0)
    , m_maxSize(0)
    , m_remain(0)
    , m_received(0)
{
}

GtRecvBuffer::~GtRecvBuffer()
{
    delete[] m_buffer;
}

int GtRecvBuffer::read(QAbstractSocket *socket, bool wait)
{
    qint64 bytesRead;

    if (m_received < sizeof(m_remain)) {
        if (wait) {
            if (GtSvcUtil::readData(socket,
                                    (char *)&m_remain + m_received,
                                    sizeof(m_remain) - m_received))
            {
                bytesRead = sizeof(m_remain) - m_received;
            }
            else {
                bytesRead = -1;
            }
        }
        else {
            bytesRead = socket->read((char *)&m_remain + m_received,
                                     sizeof(m_remain) - m_received);
        }

        if (bytesRead < 0)
            return ReadError;

        m_received += bytesRead;
        if (m_received < sizeof(m_remain))
            return 0;

        m_remain = qFromBigEndian<quint16>((const uchar*)&m_remain);
        if (m_bufferSize < m_remain) {
            if (m_maxSize > 0 && m_remain > m_maxSize)
                return ReadError;

            delete[] m_buffer;
            m_buffer = new char[m_remain];
            m_bufferSize = m_remain;
        }
    }

    if (wait) {
        if (GtSvcUtil::readData(socket,
                                m_buffer + m_received -
                                sizeof(m_remain), m_remain))
        {
            bytesRead = m_remain;
        }
        else {
            bytesRead = -1;
        }
    }
    else {
        bytesRead = socket->read(m_buffer + m_received -
                                 sizeof(m_remain), m_remain);
    }

    if (bytesRead < 0)
        return ReadError;

    m_remain -= bytesRead;
    m_received += bytesRead;
    if (0 == m_remain)
        return ReadMessage;

    return 0;
}

GT_END_NAMESPACE
