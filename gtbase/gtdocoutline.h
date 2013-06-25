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
    class GT_BASE_EXPORT Node
    {
    public:
        explicit Node() : page(-1), row(-1), _next(0), _parent(0), _child(0), _childCount(0) {}
        Node(const QString &t, int p, int r) : title(t), page(p), row(r), _next(0), _parent(0), _child(0), _childCount(0) {}

    public:
        inline Node* next() const { return _next; }
        inline Node* parent() const { return _parent; }
        inline Node* child() const { return _child; }
        inline int childCount() const { return _childCount; }
        Node* child(int row) const;

    public:
        const QString title;
        const int page;
        const int row;

    private:
        friend class GtDocOutlinePrivate;
        Node *_next;
        Node *_parent;
        Node *_child;
        int _childCount;
    };

public:
    explicit GtDocOutline(GtAbstractOutline *ao, QObject *parent = 0);
    ~GtDocOutline();

public:
    Node* first() const;

protected:
    QScopedPointer<GtDocOutlinePrivate> d_ptr;

private:
    Q_DISABLE_COPY(GtDocOutline)
    Q_DECLARE_PRIVATE(GtDocOutline)
};

#ifndef QT_NO_DEBUG_STREAM
GT_BASE_EXPORT QDebug operator<<(QDebug, const GtDocOutline::Node &);
#endif

GT_END_NAMESPACE

#endif  /* __GT_DOC_OUTLINE_H__ */
