/*
 * Copyright (C) 2013 Tom Wong. All rights reserved.
 */
#ifndef __GT_FT_CLIENT_H__
#define __GT_FT_CLIENT_H__

#include "gtobject.h"
#include <QtCore/QObject>

class QIODevice;

GT_BEGIN_NAMESPACE

class GtFTClientPrivate;

class GT_SVCE_EXPORT GtFTClient : public QObject, public GtObject
{
    Q_OBJECT

public:
    explicit GtFTClient(QObject *parent = 0);
    ~GtFTClient();

public:
    void upload(const char *session, QIODevice *device,
                qint64 offset, qint64 size);
    void download(const char *session, QIODevice *device,
                  qint64 offset, qint64 size);

private:
    QScopedPointer<GtFTClientPrivate> d_ptr;

private:
    Q_DISABLE_COPY(GtFTClient)
    Q_DECLARE_PRIVATE(GtFTClient)
};

GT_END_NAMESPACE

#endif  /* __GT_FT_CLIENT_H__ */
