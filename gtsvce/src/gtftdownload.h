/*
 * Copyright (C) 2013 Tom Wong. All rights reserved.
 */
#ifndef __GT_FT_DOWNLOAD_H__
#define __GT_FT_DOWNLOAD_H__

#include "gtobject.h"
#include <QtCore/QObject>
#include <QtCore/QRunnable>

GT_BEGIN_NAMESPACE

class GtFTDownloadPrivate;

class GT_SVCE_EXPORT GtFTDownload : public QObject, public GtObject, public QRunnable
{
    Q_OBJECT

public:
    explicit GtFTDownload(QObject *parent = 0);
    ~GtFTDownload();

public:
    void run();

private:
    QScopedPointer<GtFTDownloadPrivate> d_ptr;

private:
    Q_DISABLE_COPY(GtFTDownload)
    Q_DECLARE_PRIVATE(GtFTDownload)
};

GT_END_NAMESPACE

#endif  /* __GT_FT_DOWNLOAD_H__ */
