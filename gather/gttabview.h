/*
 * Copyright (C) 2013 Tom Wong. All rights reserved.
 */
#ifndef __GT_TAB_VIEW_H__
#define __GT_TAB_VIEW_H__

#include "gtobject.h"
#include <QtWidgets/QWidget>

GT_BEGIN_NAMESPACE

class GtMainWindow;

class GtTabView : public QWidget, public GtObject
{
    Q_OBJECT

public:
    explicit GtTabView(QWidget *parent = 0);
    ~GtTabView();

public Q_SLOTS:
    virtual void onCut();
    virtual void onCopy();
    virtual void onPaste();
    virtual void onDelete();
    virtual void onZoomIn();
    virtual void onZoomOut();
    virtual void onRotateLeft();
    virtual void onRotateRight();

protected:
    virtual void gainCurrent();
    virtual void loseCurrent();
    virtual void mainWindowClose(GtTabView *current);

private:
    friend class GtMainWindow;
    Q_DISABLE_COPY(GtTabView)

protected:
    GtMainWindow *m_mainWindow;
};

GT_END_NAMESPACE

#endif  /* __GT_TAB_VIEW_H__ */
