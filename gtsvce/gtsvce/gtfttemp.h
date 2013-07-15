/*
 * Copyright (C) 2013 Tom Wong. All rights reserved.
 */
#ifndef __GT_FT_TEMP_H__
#define __GT_FT_TEMP_H__

#include "gtobject.h"
#include <QtCore/QIODevice>

GT_BEGIN_NAMESPACE

class GtFTTempData;
class GtFTTempPrivate;

class GT_SVCE_EXPORT GtFTTemp : public QIODevice, public GtObject
{
    Q_OBJECT

public:
    explicit GtFTTemp(QObject *parent = 0);
    explicit GtFTTemp(const QString &dir,
                      const QString &fileId,
                      QObject *parent = 0);
    ~GtFTTemp();

public:
    void setPath(const QString &dir, const QString &fileId);
    QString fileId() const;
    QString metaPath() const;
    QString dataPath() const;

    bool open(OpenMode mode);
    void close();

    bool flush();
    qint64 size() const;
    bool seek(qint64 pos);

    int temps_size() const;
    const GtFTTempData& temps(int index) const;

    qint64 complete(qint64 begin = 0) const;
    bool exists() const;
    bool remove();

public:
    static qint64 complete(const QList<GtFTTempData*> &list, qint64 begin);
    static void append(QList<GtFTTempData*> &list, qint64 begin, qint64 end);

protected:
    qint64 readData(char *data, qint64 maxlen);
    qint64 writeData(const char *data, qint64 len);

private:
    QScopedPointer<GtFTTempPrivate> d_ptr;

private:
    Q_DISABLE_COPY(GtFTTemp)
    Q_DECLARE_PRIVATE(GtFTTemp)
};

GT_END_NAMESPACE

#endif  /* __GT_FT_TEMP_H__ */
