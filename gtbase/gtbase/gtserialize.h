/*
 * Copyright (C) 2013 Tom Wong. All rights reserved.
 */
#ifndef __GT_SERIALIZE_H__
#define __GT_SERIALIZE_H__

#include "gtcommon.h"
#include <QtCore/QByteArray>

GT_BEGIN_NAMESPACE

class GtSerialize
{
public:
    template<typename TS, typename TD>
    static bool serialize(const TS &src, QByteArray &data);

    template<typename TS, typename TD>
    static bool deserialize(TS &src, const QByteArray &data);

    template<typename TS, typename TD>
    static QDataStream &serialize(QDataStream &stream, const TS &src);

    template<typename TS, typename TD>
    static QDataStream &deserialize(QDataStream &stream, TS &src);
};

template<typename TS, typename TD>
bool GtSerialize::serialize(const TS &src, QByteArray &data)
{
    TD dest;
    src.serialize(dest);
    data.resize(dest.ByteSize());
    return dest.SerializeToArray(data.data(), data.size());
}

template<typename TS, typename TD>
bool GtSerialize::deserialize(TS &src, const QByteArray &data)
{
    TD dest;
    if (!dest.ParseFromArray(data.data(), data.size()))
        return false;

    return src.deserialize(dest);
}

template<typename TS, typename TD>
QDataStream &GtSerialize::serialize(QDataStream &stream, const TS &src)
{
    QByteArray data;
    if (serialize<TS, TD>(src, data))
        stream << data;
    return stream;
}

template<typename TS, typename TD>
QDataStream &GtSerialize::deserialize(QDataStream &stream, TS &src)
{
    QByteArray data;
    stream >> data;
    deserialize<TS, TD>(src, data);
    return stream;
}

GT_END_NAMESPACE

#endif  /* __GT_SERIALIZE_H__ */
