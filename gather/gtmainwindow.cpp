/*
 * Copyright (C) 2013 Tom Wong. All rights reserved.
 */
#include "gtmainwindow.h"
#include "gtdocloader.h"
#include "gtdocmodel.h"
#include "gtdocoutline.h"
#include "gtdocpage.h"
#include "gtdocument.h"
#include "gtdocview.h"
#include <QtCore/QtDebug>
#include <QtCore/QSettings>
#include <QtCore/QThread>
#include <QtGui/QCloseEvent>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QLabel>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QToolBar>

GT_BEGIN_NAMESPACE

GtMainWindow::GtMainWindow()
{
    // document thread
    docLoader = QSharedPointer<GtDocLoader>(new GtDocLoader());
    docModel = QSharedPointer<GtDocModel>(new GtDocModel());
    docModel->setMinScale(0.1);
    docModel->setMaxScale(4.0);
    docModel->setMouseMode(GtDocModel::SelectText);
    docThread = new QThread(this);

    QDir dir(QCoreApplication::applicationDirPath());

    if (dir.cd("loader")) {
        int count = docLoader->registerLoaders(dir.absolutePath());
        qDebug() << "registered loaders:" << count;
    }

    docThread->start();

    // GUI thread
    docView = new GtDocView(docModel.data(), this);
    docView->setRenderThread(docThread);
    docView->setRenderCacheSize(1024 * 1024 * 20);

    setCentralWidget(docView);

    createActions();
    createMenus();
    createContextMenu();
    createToolBars();
    createStatusBar();
    readSettings();
    setWindowIcon(QIcon(":/images/logo.bmp"));
    setCurrentFile("");
}

GtMainWindow::~GtMainWindow()
{
    docView->setModel(0);
    document.clear();
    docModel.clear();
    docLoader.clear();

    docThread->quit();
    docThread->wait();
}

void GtMainWindow::createActions()
{
    // file
    openAction = new QAction(tr("&Open"), this);
    // openAction->setIcon(QIcon(":/images/open.png"));
    openAction->setShortcut(QKeySequence::Open);
    openAction->setStatusTip(tr("Open a document"));
    connect(openAction, SIGNAL(triggered()), this, SLOT(open()));

    for (int i = 0; i < MaxRecentFiles; ++i) {
        recentFileActions[i] = new QAction(this);
        recentFileActions[i]->setVisible(false);
        connect(recentFileActions[i], SIGNAL(triggered()),
                this, SLOT(openRecentFile()));
    }

    exitAction = new QAction(tr("E&xit"), this);
    exitAction->setShortcut(tr("Ctrl+Q"));
    exitAction->setStatusTip(tr("Exit the application"));
    connect(exitAction, SIGNAL(triggered()), this, SLOT(close()));

    // edit
    cutAction = new QAction(tr("Cu&t"), this);
    cutAction->setShortcut(QKeySequence::Cut);
    cutAction->setStatusTip(tr("Cut"));

    copyAction = new QAction(tr("Cop&y"), this);
    copyAction->setShortcut(QKeySequence::Copy);
    copyAction->setStatusTip(tr("Copy"));

    pasteAction = new QAction(tr("P&aste"), this);
    pasteAction->setShortcut(QKeySequence::Paste);
    pasteAction->setStatusTip(tr("Paste"));

    deleteAction = new QAction(tr("&Delete"), this);
    deleteAction->setShortcut(QKeySequence::Delete);
    deleteAction->setStatusTip(tr("Delete"));

    findAction = new QAction(tr("Find"), this);
    findAction->setShortcut(QKeySequence::Find);
    findAction->setStatusTip(tr("Find"));

    // view
    zoomInAction = new QAction(tr("Zoom &In"), this);
    if (1) {
        QList<QKeySequence> shortcuts;
        shortcuts.append(QKeySequence("Ctrl++"));
        shortcuts.append(QKeySequence("Ctrl+="));
        zoomInAction->setShortcuts(shortcuts);
    }
    else {
        //zoomInAction->setShortcut(QKeySequence::ZoomIn);
    }
    zoomInAction->setStatusTip(tr("Zoom In"));
    connect(zoomInAction, SIGNAL(triggered()), this, SLOT(zoomIn()));

    zoomOutAction = new QAction(tr("Zoom &Out"), this);
    zoomOutAction->setShortcut(QKeySequence::ZoomOut);
    zoomOutAction->setStatusTip(tr("Zoom Out"));
    connect(zoomOutAction, SIGNAL(triggered()), this, SLOT(zoomOut()));

    rotateLeftAction = new QAction(tr("Rotate &Left"), this);
    rotateLeftAction->setShortcut(QKeySequence("Ctrl+Left"));
    rotateLeftAction->setStatusTip(tr("Rotate Left"));
    connect(rotateLeftAction, SIGNAL(triggered()), this, SLOT(rotateLeft()));

    rotateRightAction = new QAction(tr("Rotate &Right"), this);
    rotateRightAction->setShortcut(QKeySequence("Ctrl+Right"));
    rotateRightAction->setStatusTip(tr("Rotate Right"));
    connect(rotateRightAction, SIGNAL(triggered()), this, SLOT(rotateRight()));

    // help
    aboutAction = new QAction(tr("About &Gather"), this);
    aboutAction->setStatusTip(tr("Show the Qt library's About box"));
    connect(aboutAction, SIGNAL(triggered()), qApp, SLOT(aboutQt()));
}

void GtMainWindow::createMenus()
{
    fileMenu = menuBar()->addMenu(tr("&File"));
    fileMenu->addAction(openAction);
    separatorAction = fileMenu->addSeparator();

    for (int i = 0; i < MaxRecentFiles; ++i)
        fileMenu->addAction(recentFileActions[i]);

    fileMenu->addSeparator();
    fileMenu->addAction(exitAction);

    editMenu = menuBar()->addMenu(tr("&Edit"));
    editMenu->addAction(cutAction);
    editMenu->addAction(copyAction);
    editMenu->addAction(pasteAction);
    editMenu->addAction(deleteAction);
    editMenu->addSeparator();
    editMenu->addAction(findAction);

    viewMenu = menuBar()->addMenu(tr("&View"));
    viewMenu->addAction(zoomInAction);
    viewMenu->addAction(zoomOutAction);
    viewMenu->addSeparator();
    QMenu *menu = viewMenu->addMenu(tr("Ori&entation"));
    menu->addAction(rotateLeftAction);
    menu->addAction(rotateRightAction);

    helpMenu = menuBar()->addMenu(tr("&Help"));
    helpMenu->addAction(aboutAction);
}

void GtMainWindow::createContextMenu()
{
}

void GtMainWindow::createToolBars()
{
    fileToolBar = addToolBar(tr("&File"));
    fileToolBar->addAction(openAction);

    editToolBar = addToolBar(tr("&Edit"));
    editToolBar->addAction(cutAction);
    editToolBar->addAction(copyAction);
    editToolBar->addAction(pasteAction);
    editToolBar->addSeparator();
    editToolBar->addAction(findAction);
}

void GtMainWindow::createStatusBar()
{
    locationLabel = new QLabel(" W999 ");
    locationLabel->setAlignment(Qt::AlignHCenter);
    locationLabel->setMinimumSize(locationLabel->sizeHint());

    formulaLabel = new QLabel;
    formulaLabel->setIndent(3);
    statusBar()->addWidget(locationLabel);
    statusBar()->addWidget(formulaLabel, 1);

    updateStatusBar();
}

void GtMainWindow::updateStatusBar()
{
}

void GtMainWindow::docLoaded(GtDocument *doc)
{
    if (doc != document.data()) {
        qWarning() << "invalid document loaded:" << document.data() << doc;
        return;
    }

    if (document->isLoaded()) {
        docModel->setDocument(document.data());

        GtDocOutline *outline = document->outline();
        GtDocOutline::Iterator it(*outline);
        while (it.hasNext()) {
            qDebug() << it.next();
        }
    }
    else {
        Q_ASSERT(0);
    }
}

void GtMainWindow::open()
{
    if (okToContinue()) {
        QString fileName = QFileDialog::getOpenFileName(
            this, tr("Open Document"), lastOpenPath,
            tr("Document Files (*.pdf *.txt);;All Files (*.*)"));

        if (fileName.isEmpty())
            return;

        lastOpenPath = QFileInfo(fileName).path();
        loadFile(fileName);
    }
}

bool GtMainWindow::loadFile(const QString &fileName)
{
    docModel->setDocument(0);

    GtDocument *doc = docLoader->loadDocument(fileName, docThread);
    if (NULL == doc)
        return false;

    document = QSharedPointer<GtDocument>(doc, &QObject::deleteLater);
    Q_ASSERT(document->thread() == docThread);

    if (document->isLoaded()) {
        docLoaded(doc);
    }
    else {
        connect(doc, SIGNAL(loaded(GtDocument*)), this, SLOT(docLoaded(GtDocument*)));
    }

    setCurrentFile(fileName);
    statusBar()->showMessage(tr("File loaded"), 2000);
    return true;
}

bool GtMainWindow::okToContinue()
{
    if (isWindowModified()) {
        int r = QMessageBox::warning(this, tr("Spreadsheet"),
                                     tr("The document has been modified.\n"
                                        "Do you want to save your changes?"),
                                     QMessageBox::Yes | QMessageBox::No
                                     | QMessageBox::Cancel);
        if (r == QMessageBox::Yes) {
            return true;
        } else if (r == QMessageBox::Cancel) {
            return false;
        }
    }
    return true;
}

void GtMainWindow::closeEvent(QCloseEvent *event)
{
    if (okToContinue()) {
        writeSettings();
        event->accept();
    } else {
        event->ignore();
    }
}

void GtMainWindow::setCurrentFile(const QString &fileName)
{
    curFile = fileName;
    setWindowModified(false);

    QString shownName = tr("gather");
    if (!curFile.isEmpty()) {
        shownName = strippedName(curFile);
        recentFiles.removeAll(curFile);
        recentFiles.prepend(curFile);
        updateRecentFileActions();
    }

    setWindowTitle(shownName);
}

QString GtMainWindow::strippedName(const QString &fullFileName)
{
    return QFileInfo(fullFileName).fileName();
}

void GtMainWindow::updateRecentFileActions()
{
    QMutableStringListIterator i(recentFiles);
    while (i.hasNext()) {
        if (!QFile::exists(i.next()))
            i.remove();
    }
    for (int j = 0; j < MaxRecentFiles; ++j) {
        if (j < recentFiles.count()) {
            QString text = tr("&%1 %2")
                           .arg(j + 1)
                           .arg(strippedName(recentFiles[j]));
            recentFileActions[j]->setText(text);
            recentFileActions[j]->setData(recentFiles[j]);
            recentFileActions[j]->setVisible(true);
        } else {
            recentFileActions[j]->setVisible(false);
        }
    }
    separatorAction->setVisible(!recentFiles.isEmpty());
}

void GtMainWindow::openRecentFile()
{
    if (okToContinue()) {
        QAction *action = qobject_cast<QAction *>(sender());
        if (action)
            loadFile(action->data().toString());
    }
}

void GtMainWindow::find()
{
}

void GtMainWindow::zoomIn()
{
    docView->zoomIn();
}

void GtMainWindow::zoomOut()
{
    docView->zoomOut();
}

void GtMainWindow::rotateLeft()
{
    docModel->setRotation(docModel->rotation() - 90);
}

void GtMainWindow::rotateRight()
{
    docModel->setRotation(docModel->rotation() + 90);
}

void GtMainWindow::about()
{
    QMessageBox::about(this, tr("About Spreadsheet"),
                       tr("<h2>Spreadsheet 1.1</h2>"
                          "<p>Copyright &copy; 2008 Software Inc."
                          "<p>Spreadsheet is a small application that "
                          "demonstrates QAction, QMainWindow, QMenuBar, "
                          "QStatusBar, QTableWidget, QToolBar, and many other "
                          "Qt classes."));
}

void GtMainWindow::writeSettings()
{
    QSettings settings("Software Inc.", "Spreadsheet");
    settings.setValue("geometry", saveGeometry());
    settings.setValue("recentFiles", recentFiles);
    settings.setValue("lastOpenPath", lastOpenPath);
}

void GtMainWindow::readSettings()
{
    QSettings settings("Software Inc.", "Spreadsheet");
    restoreGeometry(settings.value("geometry").toByteArray());
    recentFiles = settings.value("recentFiles").toStringList();
    lastOpenPath = settings.value("lastOpenPath").toString();
    updateRecentFileActions();
}

GT_END_NAMESPACE
