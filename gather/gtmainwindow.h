/*
 * Copyright (C) 2013 Tom Wong. All rights reserved.
 */
#ifndef __GT_MAINWINDOW_H__
#define __GT_MAINWINDOW_H__

#include "gtcommon.h"
#include <QtWidgets/QMainWindow>

class QAction;
class QLabel;

GT_BEGIN_NAMESPACE

class GtDocLoader;
class GtDocument;
class GtDocModel;
class GtDocView;

class GtMainWindow : public QMainWindow
{
    Q_OBJECT;

public:
    GtMainWindow();
    ~GtMainWindow();

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
    // Objects in document thread
    QThread *docThread;
    QSharedPointer<GtDocLoader> docLoader;
    QSharedPointer<GtDocModel> docModel;
    QSharedPointer<GtDocument> document;

    // Objects in GUI thread
    GtDocView *docView;
    QLabel *locationLabel;
    QLabel *formulaLabel;
    QStringList recentFiles;
    QString curFile;
    QString lastOpenPath;

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

GT_END_NAMESPACE

#endif  /* __GT_MAINWINDOW_H__ */
