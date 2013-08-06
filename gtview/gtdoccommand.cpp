/*
 * Copyright (C) 2013 Tom Wong. All rights reserved.
 */
#include "gtdoccommand.h"
#include "gtbookmark.h"
#include "gtbookmarks.h"
#include "gtdocmodel.h"
#include "gtdocnote.h"
#include "gtdocnotes.h"
#include <QtCore/QDebug>

GT_BEGIN_NAMESPACE

GtDocCommand::GtDocCommand(GtDocModel *model)
    : m_model(model)
    , m_done(false)
{
}

GtDocCommand::~GtDocCommand()
{
}

void GtDocCommand::undo()
{
    QUndoCommand::undo();
    m_done = false;
}

void GtDocCommand::redo()
{
    m_done = true;
    QUndoCommand::redo();
}

GtAddNoteCommand::GtAddNoteCommand(GtDocModel *model, GtDocNote *note)
    : GtDocCommand(model)
    , m_note(note)
{
}

GtAddNoteCommand::~GtAddNoteCommand()
{
    if (!m_done)
        delete m_note;
}

void GtAddNoteCommand::undo()
{
    GtDocCommand::undo();
    m_model->notes()->removeNote(m_note);
}

void GtAddNoteCommand::redo()
{
    m_model->notes()->addNote(m_note);
    GtDocCommand::redo();
}

GtAddBookmarkCommand::GtAddBookmarkCommand(GtDocModel *model,
                                           GtBookmark *parent,
                                           GtBookmark *before,
                                           GtBookmark *bookmark)
    : GtDocCommand(model)
    , m_parent(parent)
    , m_before(before)
    , m_bookmark(bookmark)
{
    if (!m_parent)
        m_parent = m_model->bookmarks()->root();
}

GtAddBookmarkCommand::~GtAddBookmarkCommand()
{
    if (!m_done)
        delete m_bookmark;
}

void GtAddBookmarkCommand::undo()
{
    GtDocCommand::undo();

    GtBookmarks *bookmarks = m_model->bookmarks();

    m_parent->remove(m_bookmark);
    emit bookmarks->removed(m_bookmark);
}

void GtAddBookmarkCommand::redo()
{
    GtBookmarks *bookmarks = m_model->bookmarks();

    m_parent->insert(m_before, m_bookmark);
    emit bookmarks->added(m_bookmark);

    GtDocCommand::redo();
}

GtDelBookmarkCommand::GtDelBookmarkCommand(GtDocModel *model,
                                           GtBookmark *bookmark)
    : GtDocCommand(model)
    , m_bookmark(bookmark)
{
    m_parent = bookmark->parent();
    m_before = bookmark->next();
}

GtDelBookmarkCommand::~GtDelBookmarkCommand()
{
    if (m_done)
        delete m_bookmark;
}

void GtDelBookmarkCommand::undo()
{
    GtDocCommand::undo();

    GtBookmarks *bookmarks = m_model->bookmarks();

    m_parent->insert(m_before, m_bookmark);
    emit bookmarks->removed(m_bookmark);
}

void GtDelBookmarkCommand::redo()
{
    GtBookmarks *bookmarks = m_model->bookmarks();

    m_parent->remove(m_bookmark);
    emit bookmarks->removed(m_bookmark);

    GtDocCommand::redo();
}

GT_END_NAMESPACE
