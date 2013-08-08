/*
 * Copyright (C) 2013 Tom Wong. All rights reserved.
 */
#ifndef __GT_DOC_META_H__
#define __GT_DOC_META_H__

#include "gtobject.h"
#include <QtCore/QObject>

GT_BEGIN_NAMESPACE

class GtDocMetaMsg;
class GtDocMetaPrivate;

class GT_BASE_EXPORT GtDocMeta : public QObject, public GtSharedObject
{
    Q_OBJECT

public:
    explicit GtDocMeta(const QString &id, QObject *parent = 0);
    ~GtDocMeta();

public:
    QString id() const;
    quint32 usn() const;

    QString bookmarksId() const;
    void setBookmarksId(const QString &id);

    QString notesId() const;
    void setNotesId(const QString &id);

    void serialize(GtDocMetaMsg &msg) const;
    bool deserialize(const GtDocMetaMsg &msg);

Q_SIGNALS:
    void bookmarksIdChanged(const QString &id);
    void notesIdChanged(const QString &id);

protected:
    QScopedPointer<GtDocMetaPrivate> d_ptr;

private:
    Q_DISABLE_COPY(GtDocMeta)
    Q_DECLARE_PRIVATE(GtDocMeta)
};

#ifndef QT_NO_DATASTREAM
GT_BASE_EXPORT QDataStream &operator<<(QDataStream &, const GtDocMeta &);
GT_BASE_EXPORT QDataStream &operator>>(QDataStream &, GtDocMeta &);
#endif

GT_END_NAMESPACE

#endif  /* __GT_DOC_META_H__ */
