/*
 * Copyright (C) 2013 Tom Wong. All rights reserved.
 */
#ifndef __GT_FT_CLIENT_H__
#define __GT_FT_CLIENT_H__

#include "gtobject.h"
#include <QtCore/QIODevice>
#include <QtNetwork/QAbstractSocket>

GT_BEGIN_NAMESPACE

class GtFTClientPrivate;

class GT_SVCE_EXPORT GtFTClient : public QIODevice, public GtObject
{
    Q_OBJECT

public:
    enum ConnectionCode {
        OpenSuccess,
        InvalidSession,
        UnknownError = -1
    };

public:
    explicit GtFTClient(QObject *parent = 0);
    GtFTClient(const QString &fileId,
               const QHostAddress &address,
               quint16 port,
               const QString &session,
               QObject *parent = 0);
    ~GtFTClient();

public:
    QString fileId() const;
    void setFileInfo(const QString &fileId,
                     const QHostAddress &address,
                     quint16 port,
                     const QString &session);

    bool open(OpenMode mode);
    void close();

    bool flush();
    qint64 size() const;

    qint64 readData(char *data, qint64 maxlen);
    qint64 writeData(const char *data, qint64 len);

Q_SIGNALS:
    void connection(int result);

private Q_SLOTS:
    void handleRead();
    void handleConnected();
    void handleDisconnected();
    void handleError(QAbstractSocket::SocketError error);

private:
    QScopedPointer<GtFTClientPrivate> d_ptr;

private:
    Q_DISABLE_COPY(GtFTClient)
    Q_DECLARE_PRIVATE(GtFTClient)
};

GT_END_NAMESPACE

#endif  /* __GT_FT_CLIENT_H__ */
