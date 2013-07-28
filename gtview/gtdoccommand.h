/*
 * Copyright (C) 2013 Tom Wong. All rights reserved.
 */
#ifndef __GT_DOC_COMMAND_H__
#define __GT_DOC_COMMAND_H__

#include "gtobject.h"
#include <QtWidgets/QUndoCommand>

GT_BEGIN_NAMESPACE

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

class GT_VIEW_EXPORT GtHighlightCommand : public GtDocCommand
{
public:
    GtHighlightCommand(GtDocModel *model, const GtDocRange &range);
    ~GtHighlightCommand();

public:
    void undo();
    void redo();

protected:
    GtDocNote *m_note;
};

GT_END_NAMESPACE

#endif  /* __GT_DOC_COMMAND_H__ */
