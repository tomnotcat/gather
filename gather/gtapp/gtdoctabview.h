/*
 * Copyright (C) 2013 Tom Wong. All rights reserved.
 */
#ifndef __GT_DOC_TAB_VIEW_H__
#define __GT_DOC_TAB_VIEW_H__

#include "gttabview.h"

class QSplitter;
class QTreeView;
class QUndoStack;
class QVBoxLayout;

GT_BEGIN_NAMESPACE

class GtBookmark;
class GtDocModel;
class GtDocument;
class GtDocView;
class GtTocModel;

class GtDocTabView : public GtTabView
{
    Q_OBJECT

public:
    enum {
        MaxBookmarkTitle = 128
    };

public:
    explicit GtDocTabView(QWidget *parent = 0);
    ~GtDocTabView();

public:
    inline GtDocModel* docModel() const { return m_docModel; }
    void setDocModel(GtDocModel *docModel);

    inline QUndoStack* undoStack() const { return m_undoStack; }
    void setUndoStack(QUndoStack *undoStack);

protected:
    void gainActive();
    void loseActive();
    void saveSettings(GtMainSettings *settings);

private Q_SLOTS:
    void onDelete();
    void docViewContextMenu(const QPoint &pos);
    void tocViewContextMenu(const QPoint &pos);
    void documentLoaded(GtDocument *document);
    void gotoBookmark(const QModelIndex &index);
    void gotoBookmark();
    void addBookmark();
    void setDestination();
    void renameBookmark();
    void renameBookmark(GtBookmark *bookmark, const QString &name);
    void searchSelectedText();

private:
    // model
    GtDocModel *m_docModel;
    GtTocModel *m_tocModel;
    QUndoStack *m_undoStack;

    // view
    QVBoxLayout *m_verticalLayout;
    QSplitter *m_splitter;
    GtDocView *m_docView;
    QTreeView *m_tocView;

    // undo/redo
    QAction *m_undoAction;
    QAction *m_redoAction;
};

GT_END_NAMESPACE

#endif  /* __GT_DOC_TAB_VIEW_H__ */
