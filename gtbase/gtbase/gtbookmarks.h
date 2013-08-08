/*
 * Copyright (C) 2013 Tom Wong. All rights reserved.
 */
#ifndef __GT_BOOKMARKS_H__
#define __GT_BOOKMARKS_H__

#include "gtobject.h"
#include <QtCore/QObject>

GT_BEGIN_NAMESPACE

class GtBookmark;
class GtBookmarksMsg;
class GtBookmarksPrivate;

class GT_BASE_EXPORT GtBookmarks : public QObject, public GtSharedObject
{
    Q_OBJECT

public:
    explicit GtBookmarks(const QString &id, QObject *parent = 0);
    ~GtBookmarks();

public:
    QString id() const;
    quint32 usn() const;

    const GtBookmark* root() const;
    GtBookmark* root();

    void serialize(GtBookmarksMsg &msg) const;
    bool deserialize(const GtBookmarksMsg &msg);

Q_SIGNALS:
    void added(GtBookmark *bookmark);
    void removed(GtBookmark *bookmark);
    void updated(GtBookmark *bookmark, int flags);

protected:
    QScopedPointer<GtBookmarksPrivate> d_ptr;

private:
    Q_DISABLE_COPY(GtBookmarks)
    Q_DECLARE_PRIVATE(GtBookmarks)
};

#ifndef QT_NO_DATASTREAM
GT_BASE_EXPORT QDataStream &operator<<(QDataStream &, const GtBookmarks &);
GT_BASE_EXPORT QDataStream &operator>>(QDataStream &, GtBookmarks &);
#endif

GT_END_NAMESPACE

#endif  /* __GT_BOOKMARKS_H__ */
