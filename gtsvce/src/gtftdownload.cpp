/*
 * Copyright (C) 2013 Tom Wong. All rights reserved.
 */
#include "gtftdownload.h"
#include <QtCore/QDebug>
#include <QtNetwork/QTcpSocket>

GT_BEGIN_NAMESPACE

class GtFTDownloadPrivate
{
    Q_DECLARE_PUBLIC(GtFTDownload)

public:
    explicit GtFTDownloadPrivate(GtFTDownload *q);
    ~GtFTDownloadPrivate();

protected:
    GtFTDownload *q_ptr;
};

GtFTDownloadPrivate::GtFTDownloadPrivate(GtFTDownload *q)
    : q_ptr(q)
{
}

GtFTDownloadPrivate::~GtFTDownloadPrivate()
{
}

GtFTDownload::GtFTDownload(QObject *parent)
    : QObject(parent)
    , d_ptr(new GtFTDownloadPrivate(this))
{
}

GtFTDownload::~GtFTDownload()
{
}

void GtFTDownload::run()
{
}

GT_END_NAMESPACE
