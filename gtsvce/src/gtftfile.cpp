/*
 * Copyright (C) 2013 Tom Wong. All rights reserved.
 */
#include "gtftfile.h"
#include "gtftclient.h"
#include <QtCore/QDebug>

GT_BEGIN_NAMESPACE

class GtFTFilePrivate
{
    Q_DECLARE_PUBLIC(GtFTFile)

public:
    explicit GtFTFilePrivate(GtFTFile *q);
    ~GtFTFilePrivate();

protected:
    GtFTFile *q_ptr;
    GtFTFileHost *host;
    QString fileId;
};

GtFTFilePrivate::GtFTFilePrivate(GtFTFile *q)
    : q_ptr(q)
    , host(0)
{
}

GtFTFilePrivate::~GtFTFilePrivate()
{
}

GtFTFileHost::GtFTFileHost()
{
}

GtFTFileHost::~GtFTFileHost()
{
}

GtFTFile::GtFTFile(GtFTFileHost *host, const QString &fileId)
    : d_ptr(new GtFTFilePrivate(this))
{
    d_ptr->host = host;
    d_ptr->fileId = fileId;
}

GtFTFile::~GtFTFile()
{
}

QString GtFTFile::fileId() const
{
    Q_D(const GtFTFile);
    return d->fileId;
}

void GtFTFile::close()
{
    Q_D(GtFTFile);
    d->host->closeFile(this);
}

GT_END_NAMESPACE
