/*
 * Copyright (C) 2013 Tom Wong. All rights reserved.
 */
#ifndef __GT_TAB_VIEW_H__
#define __GT_TAB_VIEW_H__

#include "gtobject.h"
#include <QtWidgets/QWidget>

GT_BEGIN_NAMESPACE

class GtMainWindow;
class GtMainSettings;

class GT_APP_EXPORT GtTabView : public QWidget, public GtObject
{
    Q_OBJECT

public:
    explicit GtTabView(QWidget *parent = 0);
    ~GtTabView();

public:
    inline GtMainWindow* mainWindow() const { return m_mainWindow; }
    inline bool isActive() const { return m_active; }

protected:
    virtual void gainActive();
    virtual void loseActive();
    virtual void saveSettings(GtMainSettings *settings);

private:
    friend class GtMainWindow;
    Q_DISABLE_COPY(GtTabView)

private:
    GtMainWindow *m_mainWindow;
    bool m_active;
};

GT_END_NAMESPACE

#endif  /* __GT_TAB_VIEW_H__ */
