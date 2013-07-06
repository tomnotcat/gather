/*
 * Copyright (C) 2013 Tom Wong. All rights reserved.
 */
#ifndef __GT_FT_UPLOAD_H__
#define __GT_FT_UPLOAD_H__

#include "gtobject.h"
#include <QtCore/QObject>
#include <QtCore/QRunnable>

class QIODevice;
class QHostAddress;
class QAbstractSocket;

GT_BEGIN_NAMESPACE

class GtFTUploadPrivate;

class GT_SVCE_EXPORT GtFTUpload : public QObject, public GtObject, public QRunnable
{
    Q_OBJECT

public:
    enum UploadError {
        ConnectionError,
        SendError,
        DataError,
        AuthError,
        UnknownError = -1
    };

public:
    GtFTUpload(const QHostAddress &address, quint16 port,
               const QString &session, const QString &secret,
               QIODevice *device, QObject *parent = 0);
    ~GtFTUpload();

public:
    void run();

Q_SIGNALS:
    void error(UploadError error);

protected:
    bool initUpload(QAbstractSocket *socket);

private:
    QScopedPointer<GtFTUploadPrivate> d_ptr;

private:
    Q_DISABLE_COPY(GtFTUpload)
    Q_DECLARE_PRIVATE(GtFTUpload)
};

GT_END_NAMESPACE

#endif  /* __GT_FT_UPLOAD_H__ */
