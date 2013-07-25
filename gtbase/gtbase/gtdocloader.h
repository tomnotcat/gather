/*
 * Copyright (C) 2013 Tom Wong. All rights reserved.
 */
#ifndef __GT_DOC_LOADER_H__
#define __GT_DOC_LOADER_H__

#include "gtobject.h"
#include <QtCore/qobject.h>

GT_BEGIN_NAMESPACE

class GtDocument;
class GtDocLoaderPrivate;

class GT_BASE_EXPORT GtDocLoader : public QObject, public GtObject
{
    Q_OBJECT

public:
    struct LoaderInfo
    {
        QString module;
        QString description;
        QString mimetype;
        QString extension;
        QString path;
    };

public:
    explicit GtDocLoader(QObject *parent = 0);
    ~GtDocLoader();

public:
    int registerLoaders(const QString &loaderDir);
    QList<const LoaderInfo *> loaderInfos();
    GtDocument* loadDocument(const QString &fileName, QThread *thread = 0);

private:
    QScopedPointer<GtDocLoaderPrivate> d_ptr;

private:
    Q_DISABLE_COPY(GtDocLoader)
    Q_DECLARE_PRIVATE(GtDocLoader)
};

GT_END_NAMESPACE

#endif  /* __GT_DOC_LOADER_H__ */
