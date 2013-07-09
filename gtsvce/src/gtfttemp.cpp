/*
 * Copyright (C) 2013 Tom Wong. All rights reserved.
 */
#include "gtfttemp.h"
#include <QtCore/QDebug>
#include <QtCore/QDir>

GT_BEGIN_NAMESPACE

class GtFTTempPrivate
{
    Q_DECLARE_PUBLIC(GtFTTemp)

public:
    explicit GtFTTempPrivate(GtFTTemp *q);
    ~GtFTTempPrivate();

protected:
    GtFTTemp *q_ptr;
    QString metaPath;
    QString dataPath;
};

GtFTTempPrivate::GtFTTempPrivate(GtFTTemp *q)
    : q_ptr(q)
{
}

GtFTTempPrivate::~GtFTTempPrivate()
{
}

GtFTTemp::GtFTTemp(QObject *parent)
    : QObject(parent)
    , d_ptr(new GtFTTempPrivate(this))
{
}

GtFTTemp::GtFTTemp(const QString &dir,
                   const QString &fileId,
                   QObject *parent)
    : QObject(parent)
    , d_ptr(new GtFTTempPrivate(this))
{
    setPath(dir, fileId);
}

GtFTTemp::~GtFTTemp()
{
}

void GtFTTemp::setPath(const QString &dir, const QString &fileId)
{
    Q_D(GtFTTemp);

    QDir path(dir);
    d->metaPath = path.filePath(fileId + ".meta");
    d->dataPath = path.filePath(fileId + ".data");
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

bool GtFTTemp::check()
{
    return false;
}

GT_END_NAMESPACE
