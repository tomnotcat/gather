/*
 * Copyright (C) 2013 Tom Wong. All rights reserved.
 */
#ifndef __GT_FT_FILE_H__
#define __GT_FT_FILE_H__

#include "gtobject.h"
#include <QtCore/QObject>

class QAbstractSocket;

GT_BEGIN_NAMESPACE

class GtFTFile;
class GtFTFilePrivate;

class GT_SVCE_EXPORT GtFTFileHost
{
public:
    explicit GtFTFileHost();
    virtual ~GtFTFileHost();

public:
    virtual void closeFile(GtFTFile *file) = 0;
    virtual QAbstractSocket* fileSocket() = 0;
};

class GT_SVCE_EXPORT GtFTFile : public QObject, public GtObject
{
    Q_OBJECT

public:
    GtFTFile(GtFTFileHost *host, const QString &fileId);
    ~GtFTFile();

public:
    QString fileId() const;
    qint64 size() const;
    void read(qint64 offset, qint64 length);
    void write(qint64 offset, qint64 length);
    void close();

Q_SIGNALS:
    void readRequest(qint64 offset, qint64 length);
    void writeRequest(qint64 offset, qint64 length);

private:
    QScopedPointer<GtFTFilePrivate> d_ptr;

private:
    Q_DISABLE_COPY(GtFTFile)
    Q_DECLARE_PRIVATE(GtFTFile)
};

GT_END_NAMESPACE

#endif  /* __GT_FT_FILE_H__ */
