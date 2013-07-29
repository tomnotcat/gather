/*
 * Copyright (C) 2013 Tom Wong. All rights reserved.
 */
#include "gtdoccommand.h"
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

GtNoteCommand::GtNoteCommand(GtDocModel *model, GtDocNote *note)
    : GtDocCommand(model)
    , m_note(note)
{
}

GtNoteCommand::~GtNoteCommand()
{
    if (!m_done)
        delete m_note;
}

void GtNoteCommand::undo()
{
    GtDocCommand::undo();
    m_model->notes()->removeNote(m_note);
}

void GtNoteCommand::redo()
{
    m_model->notes()->addNote(m_note);
    GtDocCommand::redo();
}

GT_END_NAMESPACE
