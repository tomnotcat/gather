/*
 * Copyright (C) 2013 Tom Wong. All rights reserved.
 */
#ifndef __GT_BOOKMARK_MANAGER_H__
#define __GT_BOOKMARK_MANAGER_H__

#include "gtobject.h"
#include <QtCore/QObject>

GT_BEGIN_NAMESPACE

class GtBookmarks;
class GtBookmarkManagerPrivate;

class GtBookmarkManager : public QObject, public GtObject
{
    Q_OBJECT;

public:
    explicit GtBookmarkManager(QObject *parent = 0);
    ~GtBookmarkManager();

public:
    GtBookmarks *loadBookmarks(const QString &id);

private:
    QScopedPointer<GtBookmarkManagerPrivate> d_ptr;

private:
    Q_DISABLE_COPY(GtBookmarkManager)
    Q_DECLARE_PRIVATE(GtBookmarkManager)
};

GT_END_NAMESPACE

#endif  /* __GT_BOOKMARK_MANAGER_H__ */
