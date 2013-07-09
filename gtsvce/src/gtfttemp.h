/*
 * Copyright (C) 2013 Tom Wong. All rights reserved.
 */
#ifndef __GT_FT_TEMP_H__
#define __GT_FT_TEMP_H__

#include "gtobject.h"
#include <QtCore/QObject>

GT_BEGIN_NAMESPACE

class GtFTTempPrivate;

class GT_SVCE_EXPORT GtFTTemp : public QObject, public GtObject
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
    QString metaPath() const;
    QString dataPath() const;

    bool check();

private:
    QScopedPointer<GtFTTempPrivate> d_ptr;

private:
    Q_DISABLE_COPY(GtFTTemp)
    Q_DECLARE_PRIVATE(GtFTTemp)
};

GT_END_NAMESPACE

#endif  /* __GT_FT_TEMP_H__ */
