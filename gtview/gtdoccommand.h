/*
 * Copyright (C) 2013 Tom Wong. All rights reserved.
 */
#ifndef __GT_DOC_COMMAND_H__
#define __GT_DOC_COMMAND_H__

#include "gtobject.h"
#include <QtWidgets/QUndoCommand>

GT_BEGIN_NAMESPACE

class GtBookmark;
class GtDocModel;
class GtDocNote;
class GtDocRange;

class GT_VIEW_EXPORT GtDocCommand : public QUndoCommand, public GtObject
{
public:
    explicit GtDocCommand(GtDocModel *model);
    ~GtDocCommand();

public:
    void undo();
    void redo();

protected:
    GtDocModel *m_model;
    bool m_done;
};

class GT_VIEW_EXPORT GtAddNoteCommand : public GtDocCommand
{
public:
    GtAddNoteCommand(GtDocModel *model, GtDocNote *note);
    ~GtAddNoteCommand();

public:
    void undo();
    void redo();

protected:
    GtDocNote *m_note;
};

class GT_VIEW_EXPORT GtAddBookmarkCommand : public GtDocCommand
{
public:
    GtAddBookmarkCommand(GtDocModel *model,
                         GtBookmark *parent,
                         GtBookmark *before,
                         GtBookmark *bookmark);
    ~GtAddBookmarkCommand();

public:
    void undo();
    void redo();

protected:
    GtBookmark *m_parent;
    GtBookmark *m_before;
    GtBookmark *m_bookmark;
};

class GT_VIEW_EXPORT GtRenameBookmarkCommand : public GtDocCommand
{
public:
    GtRenameBookmarkCommand(GtDocModel *model,
                            GtBookmark *bookmark,
                            const QString &name);
    ~GtRenameBookmarkCommand();

public:
    void undo();
    void redo();

protected:
    void rename();

protected:
    GtBookmark *m_bookmark;
    QString m_name;
};

class GT_VIEW_EXPORT GtDelBookmarkCommand : public GtDocCommand
{
public:
    GtDelBookmarkCommand(GtDocModel *model,
                         GtBookmark *bookmark);
    ~GtDelBookmarkCommand();

public:
    void undo();
    void redo();

protected:
    GtBookmark *m_parent;
    GtBookmark *m_before;
    GtBookmark *m_bookmark;
};

GT_END_NAMESPACE

#endif  /* __GT_DOC_COMMAND_H__ */
