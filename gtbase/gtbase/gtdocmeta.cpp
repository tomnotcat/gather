/*
 * Copyright (C) 2013 Tom Wong. All rights reserved.
 */
#include "gtdocmeta.h"
#include <QtCore/QDebug>

GT_BEGIN_NAMESPACE

class GtDocMetaPrivate
{
    Q_DECLARE_PUBLIC(GtDocMeta)

public:
    GtDocMetaPrivate(GtDocMeta *q);
    ~GtDocMetaPrivate();

protected:
    GtDocMeta *q_ptr;
    QString m_documentId;
    QString m_bookmarksId;
    QString m_notesId;
};

GtDocMetaPrivate::GtDocMetaPrivate(GtDocMeta *q)
    : q_ptr(q)
{
}

GtDocMetaPrivate::~GtDocMetaPrivate()
{
}

GtDocMeta::GtDocMeta(const QString &documentId, QObject *parent)
    : QObject(parent)
    , d_ptr(new GtDocMetaPrivate(this))
{
    d_ptr->m_documentId = documentId;
}

GtDocMeta::~GtDocMeta()
{
}

QString GtDocMeta::documentId() const
{
    return d_ptr->m_documentId;
}

QString GtDocMeta::bookmarksId() const
{
    return d_ptr->m_bookmarksId;
}

void GtDocMeta::setBookmarksId(const QString &id)
{
    Q_D(GtDocMeta);

    if (d->m_bookmarksId != id) {
        d->m_bookmarksId = id;
        emit bookmarksIdChanged(id);
    }
}

QString GtDocMeta::notesId() const
{
    return d_ptr->m_notesId;
}

void GtDocMeta::setNotesId(const QString &id)
{
    Q_D(GtDocMeta);

    if (d->m_notesId != id) {
        d_ptr->m_notesId = id;
        emit notesIdChanged(id);
    }
}

GT_END_NAMESPACE
