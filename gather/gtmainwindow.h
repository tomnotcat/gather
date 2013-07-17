/*
 * Copyright (C) 2013 Tom Wong. All rights reserved.
 */
#ifndef __GT_MAINWINDOW_H__
#define __GT_MAINWINDOW_H__

#include "gtdocument.h"
#include "ui_gtmainwindow.h"

GT_BEGIN_NAMESPACE

class GtDocument;
class GtDocModel;
class GtTocModel;
class GtDocView;
class GtTabView;
class GtDocTabView;

class GtMainWindow : public QMainWindow, public GtObject
{
    Q_OBJECT;

public:
    GtMainWindow();
    ~GtMainWindow();

public:
    GtTabView* tabView(int index = -1);
    GtDocTabView* newDocTab();

public Q_SLOTS:
    void showTabContextMenu(const QPoint &pos);
    void closeTab(int index = -1);
    void closeOtherTabs();

private Q_SLOTS:
    void on_actionNewWindow_triggered();
    void on_actionNewTab_triggered();
    void on_actionOpenFile_triggered();
    void on_actionCloseTab_triggered();
    void on_actionQuit_triggered();
    void on_actionCut_triggered();
    void on_actionCopy_triggered();
    void on_actionPaste_triggered();
    void on_actionDelete_triggered();
    void on_actionZoomIn_triggered();
    void on_actionZoomOut_triggered();
    void on_actionRotateLeft_triggered();
    void on_actionRotateRight_triggered();
    void on_actionAboutGather_triggered();
    void openRecentFile();

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

private:
    Ui_MainWindow ui;

    // objects in GUI thread
    QStringList recentFiles;
    QString curFile;
    QString lastOpenPath;

    // recent opened files
    enum { MaxRecentFiles = 5 };
    QAction *recentFileActions[MaxRecentFiles];
    QAction *recentFileSeparator;
};

GT_END_NAMESPACE

#endif  /* __GT_MAINWINDOW_H__ */
