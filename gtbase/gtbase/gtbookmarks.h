/*
 * Copyright (C) 2013 Tom Wong. All rights reserved.
 */
#ifndef __GT_BOOKMARKS_H__
#define __GT_BOOKMARKS_H__

#include "gtobject.h"
#include <QtCore/QObject>

GT_BEGIN_NAMESPACE

class GtBookmark;
class GtBookmarksPrivate;

class GT_BASE_EXPORT GtBookmarks : public QObject, public GtObject
{
    Q_OBJECT

public:
    explicit GtBookmarks(QObject *parent = 0);
    ~GtBookmarks();

public:
    const GtBookmark* root() const;
    GtBookmark* root();

protected:
    QScopedPointer<GtBookmarksPrivate> d_ptr;

private:
    Q_DISABLE_COPY(GtBookmarks)
    Q_DECLARE_PRIVATE(GtBookmarks)
};

GT_END_NAMESPACE

#endif  /* __GT_BOOKMARKS_H__ */
