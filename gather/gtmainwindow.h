/*
 * Copyright (C) 2013 Tom Wong. All rights reserved.
 */
#ifndef __GT_MAINWINDOW_H__
#define __GT_MAINWINDOW_H__

#include "gtobject.h"
#include "ui_gtmainwindow.h"

GT_BEGIN_NAMESPACE

class GtDocLoader;
class GtDocument;
class GtDocModel;
class GtTocModel;
class GtDocView;
class GtUserClient;

#define ORGANIZATION_NAME "Clue Network"
#define APPLICATION_NAME "Gather"

class GtMainWindow : public QMainWindow, public GtObject
{
    Q_OBJECT;

public:
    GtMainWindow();
    ~GtMainWindow();

private Q_SLOTS:
    void on_actionOpen_triggered();
    void on_actionExit_triggered();
    void on_actionZoomIn_triggered();
    void on_actionZoomOut_triggered();
    void on_actionRotateLeft_triggered();
    void on_actionRotateRight_triggered();
    void on_actionAboutGather_triggered();
    void openRecentFile();
    void showDocViewContextMenu(const QPoint &pos);
    void docLoaded(GtDocument *doc);
    void tocChanged(const QModelIndex &index);
    void searchSelectedText();

private:
    void readSettings();
    void writeSettings();
    bool okToContinue();
    bool loadFile(const QString &fileName);
    void setCurrentFile(const QString &fileName);
    void updateRecentFileActions();
    QString strippedName(const QString &fullFileName);

private:
    void changeEvent(QEvent *event);
    void closeEvent(QCloseEvent *event);

private:
    Ui::MainWindow ui;

    // Objects in document thread
    QThread *docThread;
    QSharedPointer<GtDocLoader> docLoader;
    QSharedPointer<GtDocModel> docModel;
    QSharedPointer<GtDocument> document;
    QSharedPointer<GtTocModel> tocModel;

    // Network
    GtUserClient *client;

    // Objects in GUI thread
    QStringList recentFiles;
    QString curFile;
    QString lastOpenPath;

    // Recent opened files
    enum { MaxRecentFiles = 5 };
    QAction *recentFileActions[MaxRecentFiles];
    QAction *recentFileSeparator;
};

GT_END_NAMESPACE

#endif  /* __GT_MAINWINDOW_H__ */
