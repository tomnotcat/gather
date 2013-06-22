/*
 * Copyright (C) 2013 Tom Wong. All rights reserved.
 */
#ifndef __GT_DOC_OUTLINE_H__
#define __GT_DOC_OUTLINE_H__

#include "gtcommon.h"
#include <QtCore/QObject>

GT_BEGIN_NAMESPACE

class GtAbstractOutline;
class GtDocOutlinePrivate;

class GT_BASE_EXPORT GtDocOutline : public QObject
{
    Q_OBJECT

public:
    class Iterator;
    class GT_BASE_EXPORT Entry
    {
    public:
        explicit Entry() : page(-1), next(0), child(0) {}
        Entry(const QString &t, int p) : title(t), page(p), next(0), child(0) {}

    public:
        QString title;
        int page;

    private:
        friend class GtDocOutlinePrivate;
        friend class GtDocOutline::Iterator;
        Entry *next;
        Entry *child;
    };

    class GT_BASE_EXPORT Iterator
    {
    public:
        Iterator(const GtDocOutline &o, const Entry *e = 0);
        ~Iterator();

    public:
        bool hasNext() const { return entry != 0; }
        bool hasChild() const { return entry && entry->child != 0; }
        Iterator child() const { return Iterator(*outline, entry->child); }
        Entry next();

    private:
        const GtDocOutline *outline;
        const Entry *entry;
    };

public:
    explicit GtDocOutline(GtAbstractOutline *ao, QObject *parent = 0);
    ~GtDocOutline();

protected:
    QScopedPointer<GtDocOutlinePrivate> d_ptr;

private:
    Q_DISABLE_COPY(GtDocOutline)
    Q_DECLARE_PRIVATE(GtDocOutline)
};

#ifndef QT_NO_DEBUG_STREAM
GT_BASE_EXPORT QDebug operator<<(QDebug, const GtDocOutline::Entry &);
#endif

GT_END_NAMESPACE

#endif  /* __GT_DOC_OUTLINE_H__ */
