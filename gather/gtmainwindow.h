/*
 * Copyright (C) 2013 Tom Wong. All rights reserved.
 */
#ifndef __GT_MAINWINDOW_H__
#define __GT_MAINWINDOW_H__

#include "gtcommon.h"
#include "ui_gtmainwindow.h"

GT_BEGIN_NAMESPACE

class GtDocLoader;
class GtDocument;
class GtDocModel;
class GtDocView;

class GtMainWindow : public QMainWindow, private Ui::MainWindow
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
    void docLoaded(GtDocument *doc);

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
    // Objects in document thread
    QThread *docThread;
    QSharedPointer<GtDocLoader> docLoader;
    QSharedPointer<GtDocModel> docModel;
    QSharedPointer<GtDocument> document;

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
