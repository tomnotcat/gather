/*
 * Copyright (C) 2013 Tom Wong. All rights reserved.
 */
#ifndef __GT_DOC_LOADER_H__
#define __GT_DOC_LOADER_H__

#include "gtcommon.h"
#include <QtCore/qobject.h>

GT_BEGIN_NAMESPACE

class GtDocument;
class GtDocLoaderPrivate;

class GT_BASE_EXPORT GtDocLoader : public QObject
{
    Q_OBJECT

public:
    explicit GtDocLoader(QObject *parent = 0);
    ~GtDocLoader();

public:
    GtDocument* load(const QString &fileName);

private:
    QScopedPointer<GtDocLoaderPrivate> d_ptr;

private:
    Q_DISABLE_COPY(GtDocLoader)
    Q_DECLARE_PRIVATE(GtDocLoader)
};

GT_END_NAMESPACE

#endif  /* __GT_DOC_LOADER_H__ */
