/*
 * Copyright (C) 2013 Tom Wong. All rights reserved.
 */
#ifndef __GT_DOC_NOTES_H__
#define __GT_DOC_NOTES_H__

#include "gtobject.h"
#include <QtCore/QObject>

GT_BEGIN_NAMESPACE

class GtDocNote;
class GtDocNotesMsg;
class GtDocNotesPrivate;

class GT_BASE_EXPORT GtDocNotes : public QObject, public GtSharedObject
{
    Q_OBJECT

public:
    explicit GtDocNotes(const QString &id, QObject *parent = 0);
    ~GtDocNotes();

public:
    QString id() const;
    quint32 usn() const;

    int pageCount() const;
    QList<GtDocNote*> pageNotes(int page) const;
    QList<GtDocNote*> allNotes() const;

    void addNote(GtDocNote *note);
    bool removeNote(GtDocNote *note);
    void clearAll();

    void serialize(GtDocNotesMsg &msg) const;
    bool deserialize(const GtDocNotesMsg &msg);

Q_SIGNALS:
    void added(GtDocNote *note);
    void removed(GtDocNote *note);

protected:
    QScopedPointer<GtDocNotesPrivate> d_ptr;

private:
    Q_DISABLE_COPY(GtDocNotes)
    Q_DECLARE_PRIVATE(GtDocNotes)
};

#ifndef QT_NO_DATASTREAM
GT_BASE_EXPORT QDataStream &operator<<(QDataStream &, const GtDocNotes &);
GT_BASE_EXPORT QDataStream &operator>>(QDataStream &, GtDocNotes &);
#endif

GT_END_NAMESPACE

#endif  /* __GT_DOC_NOTES_H__ */
