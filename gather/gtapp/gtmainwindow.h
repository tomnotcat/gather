/*
 * Copyright (C) 2013 Tom Wong. All rights reserved.
 */
#ifndef __GT_MAINWINDOW_H__
#define __GT_MAINWINDOW_H__

#include "gtobject.h"
#include "ui_gtmainwindow.h"

GT_BEGIN_NAMESPACE

class GtTabView;

class GT_APP_EXPORT GtMainWindow : public QMainWindow, public GtObject
{
    Q_OBJECT;

public:
    GtMainWindow();
    ~GtMainWindow();

public:
    GtTabView* tabView(int index = -1);
    GtTabView* newTab();

public Q_SLOTS:
    void showTabContextMenu(const QPoint &pos);
    void closeTab(int index = -1);
    void closeOtherTabs();

private Q_SLOTS:
    void on_actionNewTab_triggered();
    void on_actionNewWindow_triggered();
    void on_actionOpenFile_triggered();
    void on_actionCloseTab_triggered();
    void on_actionQuit_triggered();
    void on_actionLogin_triggered();
    void on_actionAboutGather_triggered();
    void openRecentFile();
    void currentTabChanged(int index);
    void editMenuAboutToShow();
    void editMenuAboutToHide();
    void userStateChanged(int state, int error);

private:
    void openTab(GtTabView *tab);
    bool okToContinue();
    bool loadFile(const QString &fileName);
    void setCurrentFile(const QString &fileName);
    void updateRecentFileActions();
    QString strippedName(const QString &fullFileName);

private:
    void changeEvent(QEvent *event);
    void closeEvent(QCloseEvent *event);

public:
    Ui_MainWindow m_ui;

private:
    // objects in GUI thread
    QStringList m_recentFiles;
    QString m_curFile;
    QString m_lastOpenPath;
    GtTabView *m_oldTabView;

    // recent opened files
    enum { MaxRecentFiles = 5 };
    QAction *m_recentFileActions[MaxRecentFiles];
    QAction *m_recentFileSeparator;
};

GT_END_NAMESPACE

#endif  /* __GT_MAINWINDOW_H__ */
