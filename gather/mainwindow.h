/*
 * Copyright (C) 2013 Tom Wong. All rights reserved.
 */
#ifndef GATHER_MAINWINDOW_H
#define GATHER_MAINWINDOW_H

#include "common.h"
#include <QtWidgets/QMainWindow>

class QAction;
class QLabel;

GATHER_BEGIN_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT;

public:
    MainWindow();

protected:
    void closeEvent(QCloseEvent *event);

private Q_SLOTS:
    void open();
    void find();
    void about();
    void openRecentFile();
    void updateStatusBar();

private:
    void createActions();
    void createMenus();
    void createContextMenu();
    void createToolBars();
    void createStatusBar();
    void readSettings();
    void writeSettings();
    bool okToContinue();
    bool loadFile(const QString &fileName);
    void setCurrentFile(const QString &fileName);
    void updateRecentFileActions();
    QString strippedName(const QString &fullFileName);

private:
    QLabel *locationLabel;
    QLabel *formulaLabel;
    QStringList recentFiles;
    QString curFile;

    QMenu *fileMenu;
    QMenu *editMenu;
    QMenu *helpMenu;

    QToolBar *fileToolBar;
    QToolBar *editToolBar;

    QAction *openAction;
    enum { MaxRecentFiles = 5 };
    QAction *recentFileActions[MaxRecentFiles];
    QAction *separatorAction;

    QAction *cutAction;
    QAction *copyAction;
    QAction *pasteAction;
    QAction *deleteAction;
    QAction *findAction;

    QAction *aboutAction;
    QAction *exitAction;
};

GATHER_END_NAMESPACE

#endif  /* GATHER_MAINWINDOW_H */
