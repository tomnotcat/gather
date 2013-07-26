/*
 * Copyright (C) 2013 Tom Wong. All rights reserved.
 */
#ifndef __GT_DOC_TAB_VIEW_H__
#define __GT_DOC_TAB_VIEW_H__

#include "gttabview.h"
#include "gtdocmodel.h"

class QSplitter;
class QTreeView;
class QVBoxLayout;

GT_BEGIN_NAMESPACE

class GtDocModel;
class GtDocView;
class GtTocModel;

class GtDocTabView : public GtTabView
{
    Q_OBJECT

public:
    explicit GtDocTabView(QWidget *parent = 0);
    ~GtDocTabView();

public:
    GtDocModelPointer docModel() const;
    void setDocModel(GtDocModelPointer docModel);

public Q_SLOTS:
    void onCut();
    void onCopy();
    void onPaste();
    void onDelete();
    void onZoomIn();
    void onZoomOut();
    void onRotateLeft();
    void onRotateRight();

protected:
    void currentChanged(GtTabView *old, GtTabView *now);
    void mainWindowClose(GtTabView *current);

private Q_SLOTS:
    void showDocViewContextMenu(const QPoint &pos);
    void documentLoaded(GtDocument *document);
    void tocChanged(const QModelIndex &index);
    void addBookmark();
    void searchSelectedText();

private:
    // model
    GtDocModelPointer m_docModel;
    GtTocModel *m_tocModel;

    // view
    QVBoxLayout *m_verticalLayout;
    QSplitter *m_splitter;
    GtDocView *m_docView;
    QTreeView *m_tocView;
};

GT_END_NAMESPACE

#endif  /* __GT_DOC_TAB_VIEW_H__ */
