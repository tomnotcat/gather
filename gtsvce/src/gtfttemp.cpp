/*
 * Copyright (C) 2013 Tom Wong. All rights reserved.
 */
#include "gtfttemp.h"
#include "gtftmessage.pb.h"
#include <QtCore/QDebug>
#include <QtCore/QDir>

GT_BEGIN_NAMESPACE

class GtFTTempPrivate
{
    Q_DECLARE_PUBLIC(GtFTTemp)

public:
    explicit GtFTTempPrivate(GtFTTemp *q);
    ~GtFTTempPrivate();

public:
    bool saveMeta();

protected:
    GtFTTemp *q_ptr;
    QString fileId;
    QString metaPath;
    QString dataPath;
    QFile metaFile;
    QFile dataFile;
    GtFTTempMeta tempMeta;
    QList<GtFTTempData*> tempDatas;
};

GtFTTempPrivate::GtFTTempPrivate(GtFTTemp *q)
    : q_ptr(q)
{
}

GtFTTempPrivate::~GtFTTempPrivate()
{
    qDeleteAll(tempDatas);
}

bool GtFTTempPrivate::saveMeta()
{
    tempMeta.clear_datas();
    foreach(GtFTTempData *p, tempDatas) {
        *tempMeta.add_datas() = *p;
    }

    int size = tempMeta.ByteSize();

    QByteArray bytes(size, -1);
    char *data = bytes.data();

    if (tempMeta.SerializeToArray(data, size)) {
        if (metaFile.resize(0))
            return (metaFile.write(data, size) == size);
    }

    return false;
}

GtFTTemp::GtFTTemp(QObject *parent)
    : QIODevice(parent)
    , d_ptr(new GtFTTempPrivate(this))
{
}

GtFTTemp::GtFTTemp(const QString &dir,
                   const QString &fileId,
                   QObject *parent)
    : QIODevice(parent)
    , d_ptr(new GtFTTempPrivate(this))
{
    setPath(dir, fileId);
}

GtFTTemp::~GtFTTemp()
{
    close();
}

void GtFTTemp::setPath(const QString &dir, const QString &fileId)
{
    Q_D(GtFTTemp);

    d->fileId = fileId;

    QDir path(dir);
    d->metaPath = path.filePath(fileId + ".meta");
    d->dataPath = path.filePath(fileId + ".data");

    d->metaFile.setFileName(d->metaPath);
    d->dataFile.setFileName(d->dataPath);
}

QString GtFTTemp::fileId() const
{
    Q_D(const GtFTTemp);
    return d->fileId;
}

QString GtFTTemp::metaPath() const
{
    Q_D(const GtFTTemp);
    return d->metaPath;
}

QString GtFTTemp::dataPath() const
{
    Q_D(const GtFTTemp);
    return d->dataPath;
}

bool GtFTTemp::open(OpenMode mode)
{
    Q_D(GtFTTemp);

    if (!d->metaFile.open(QIODevice::ReadWrite))
        return false;

    if (d->metaFile.size() > 0) {
        QByteArray data = d->metaFile.readAll();

        if (d->tempMeta.ParseFromArray(data.constData(), data.size()) &&
            QString::fromUtf8(d->tempMeta.fileid().c_str()) == d->fileId)
        {
            qDeleteAll(d->tempDatas);
            d->tempDatas.clear();

            int count = d->tempMeta.datas_size();
            for (int i = 0; i < count; ++i) {
                GtFTTempData *p = new GtFTTempData(d->tempMeta.datas(i));
                d->tempDatas.push_back(p);
            }
        }
        else {
            mode |= QIODevice::Truncate;
            qWarning() << "invalid FTTemp meta file:" << d->metaPath;
        }
    }

    d->tempMeta.set_fileid(d->fileId.toUtf8());

    if (d->dataFile.open(mode))
        return QIODevice::open(mode);

    return false;
}

void GtFTTemp::close()
{
    if (!isOpen())
        return;

    Q_D(GtFTTemp);

    d->saveMeta();

    QIODevice::close();
    d->metaFile.close();
    d->dataFile.close();
}

bool GtFTTemp::flush()
{
    Q_D(GtFTTemp);

    if (!d->saveMeta())
        return false;

    return d->metaFile.flush() && d->dataFile.flush();
}

qint64 GtFTTemp::size() const
{
    Q_D(const GtFTTemp);
    return d->dataFile.size();
}

bool GtFTTemp::seek(qint64 pos)
{
    Q_D(GtFTTemp);

    if (!d->dataFile.seek(pos))
        return false;

    return QIODevice::seek(pos);
}

qint64 GtFTTemp::complete(qint64 begin) const
{
    Q_D(const GtFTTemp);

    if (d->tempDatas.size() == 0)
        return begin;

    GtFTTempData *p = d->tempDatas[0];
    if (p->offset() > begin)
        return begin;

    qint64 curPos;
    qint64 maxPos = p->size();
    foreach(GtFTTempData *it, d->tempDatas) {
        if (it->offset() > maxPos && it->offset() > begin)
            break;

        curPos = it->offset() + it->size();
        maxPos = MAX(curPos, maxPos);
    }

    return maxPos;
}

bool GtFTTemp::remove()
{
    Q_D(GtFTTemp);
    return d->metaFile.remove() && d->dataFile.remove();
}

qint64 GtFTTemp::readData(char *data, qint64 maxlen)
{
    Q_D(GtFTTemp);
    return d->dataFile.read(data, maxlen);
}

qint64 GtFTTemp::writeData(const char *data, qint64 len)
{
    Q_D(GtFTTemp);

    qint64 pos = d->dataFile.pos();
    qint64 size = d->dataFile.write(data, len);
    qint64 begin0 = pos;
    qint64 end0 = begin0 + size;
    bool done = false;

    foreach(GtFTTempData *p, d->tempDatas) {
        qint64 begin1 = p->offset();
        qint64 end1 = begin1 + p->size();

        if ((begin0 >= begin1 && begin0 <= end1) ||
            (end0 >= begin1 && end0 <= end1) ||
            (begin0 < begin1 && end0 > end1))
        {
            qint64 begin2 = MIN(begin0, begin1);
            qint64 end2 = MAX(end0, end1);

            p->set_offset(begin2);
            p->set_size(end2 - begin2);

            done = true;
            break;
        }
    }

    if (!done) {
        GtFTTempData *p;
        int i, count = d->tempDatas.size();

        for (i = 0; i < count; ++i) {
            p = d->tempDatas[i];
            if (pos + size < p->offset())
                break;
        }

        p = new GtFTTempData();
        p->set_offset(pos);
        p->set_size(size);
        d->tempDatas.insert(i, p);
    }

    return size;
}

GT_END_NAMESPACE
