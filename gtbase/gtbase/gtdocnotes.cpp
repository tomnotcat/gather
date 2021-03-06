/*
 * Copyright (C) 2013 Tom Wong. All rights reserved.
 */
#include "gtdocnotes.h"
#include "gtdocmessage.pb.h"
#include "gtdocnote.h"
#include "gtdocpage.h"
#include "gtdocrange.h"
#include "gtserialize.h"
#include <QtCore/QDebug>

GT_BEGIN_NAMESPACE

class GtDocNotesPrivate
{
    Q_DECLARE_PUBLIC(GtDocNotes)

public:
    explicit GtDocNotesPrivate(GtDocNotes *q);
    ~GtDocNotesPrivate();

protected:
    GtDocNotes *q_ptr;
    QString m_id;
    QList<GtDocNote*> m_notes;
    QVector<QList<GtDocNote*> > m_pages;
};

GtDocNotesPrivate::GtDocNotesPrivate(GtDocNotes *q)
    : q_ptr(q)
{
}

GtDocNotesPrivate::~GtDocNotesPrivate()
{
    qDeleteAll(m_notes);
}

GtDocNotes::GtDocNotes(const QString &id, QObject *parent)
    : QObject(parent)
    , d_ptr(new GtDocNotesPrivate(this))
{
    d_ptr->m_id = id;
}

GtDocNotes::~GtDocNotes()
{
}

QString GtDocNotes::id() const
{
    Q_D(const GtDocNotes);
    return d->m_id;
}

int GtDocNotes::pageCount() const
{
    Q_D(const GtDocNotes);
    return d->m_pages.size();
}

QList<GtDocNote*> GtDocNotes::pageNotes(int page) const
{
    Q_D(const GtDocNotes);

    if (page < d->m_pages.size())
        return d->m_pages[page];

    return QList<GtDocNote*>();
}

QList<GtDocNote*> GtDocNotes::allNotes() const
{
    Q_D(const GtDocNotes);
    return d->m_notes;
}

void GtDocNotes::addNote(GtDocNote *note)
{
    Q_D(GtDocNotes);

    // add to list
    d->m_notes.push_back(note);

    // add to pages
    GtDocRange range(note->range());
    int beginPage = range.begin().page();
    int endPage = range.end().page() + 1;

    if (endPage > d->m_pages.size())
        d->m_pages.resize(endPage);

    for (int i = beginPage; i < endPage; ++i)
        d->m_pages[i].push_back(note);

    emit added(note);
}

bool GtDocNotes::removeNote(GtDocNote *note)
{
    Q_D(GtDocNotes);

    // remove from list
    if (!d->m_notes.removeOne(note))
        return false;

    // remove from pages
    GtDocRange range(note->range());
    int beginPage = range.begin().page();
    int endPage = range.end().page() + 1;

    for (int i = beginPage; i < endPage; ++i)
        d->m_pages[i].removeOne(note);

    while (d->m_pages.size() > 0) {
        if (d->m_pages.last().size() == 0)
            d->m_pages.pop_back();
        else
            break;
    }

    emit removed(note);
    return true;
}

void GtDocNotes::clearAll()
{
    Q_D(GtDocNotes);

    qDeleteAll(d->m_notes);
    d->m_notes.clear();
    d->m_pages.clear();
}

void GtDocNotes::serialize(GtDocNotesMsg &msg) const
{
    Q_D(const GtDocNotes);

    msg.set_id(d->m_id.toUtf8());

    QList<GtDocNote*>::const_iterator it;
    for (it = d->m_notes.begin(); it != d->m_notes.end(); ++it)
        (*it)->serialize(*msg.add_notes());
}

bool GtDocNotes::deserialize(const GtDocNotesMsg &msg)
{
    Q_D(GtDocNotes);

    if (d->m_id != msg.id().c_str())
        return false;

    int count = msg.notes_size();
    for (int i = 0; i < count; ++i) {
        GtDocNote *p = new GtDocNote();

        if (p->deserialize(msg.notes(i))) {
            addNote(p);
        }
        else {
            delete p;
            return false;
        }
    }

    return true;
}

#ifndef QT_NO_DATASTREAM

QDataStream &operator<<(QDataStream &s, const GtDocNotes &n)
{
    return GtSerialize::serialize<GtDocNotes, GtDocNotesMsg>(s, n);
}

QDataStream &operator>>(QDataStream &s, GtDocNotes &n)
{
    return GtSerialize::deserialize<GtDocNotes, GtDocNotesMsg>(s, n);
}

#endif // QT_NO_DATASTREAM

GT_END_NAMESPACE
