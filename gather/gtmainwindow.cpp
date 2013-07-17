/*
 * Copyright (C) 2013 Tom Wong. All rights reserved.
 */
#include "gtmainwindow.h"
#include "gtapplication.h"
#include "gtdoctabview.h"
#include "gtmainsettings.h"
#include <QtCore/QtDebug>
#include <QtGui/QCloseEvent>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QMessageBox>

GT_BEGIN_NAMESPACE

GtMainWindow::GtMainWindow()
{
    ui.setupUi(this);

    setToolButtonStyle(Qt::ToolButtonFollowStyle);
    setAttribute(Qt::WA_DeleteOnClose, true);

    // tab widget
    QTabBar *tabBar = ui.tabWidget->tabBar();

    tabBar->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(tabBar, SIGNAL(customContextMenuRequested(const QPoint&)),
            this, SLOT(showTabContextMenu(const QPoint&)));
    connect(ui.tabWidget, SIGNAL(tabCloseRequested(int)),
            this, SLOT(closeTab(int)));
    tabBar->setSelectionBehaviorOnRemove(QTabBar::SelectPreviousTab);
    tabBar->hide();

    // adjust layout
    ui.verticalLayout->setMargin(0);

    // Recent files
    recentFileActions[0] = ui.actionRecentFile0;
    recentFileActions[1] = ui.actionRecentFile1;
    recentFileActions[2] = ui.actionRecentFile2;
    recentFileActions[3] = ui.actionRecentFile3;
    recentFileActions[4] = ui.actionRecentFile4;

    for (int i = 0; i < MaxRecentFiles; ++i) {
        recentFileActions[i]->setVisible(false);
        connect(recentFileActions[i], SIGNAL(triggered()),
                this, SLOT(openRecentFile()));
    }

    recentFileSeparator = ui.menuFile->insertSeparator(ui.actionCloseTab);

    // shortcuts
    QList<QKeySequence> shortcuts;
    shortcuts.push_back(QKeySequence("Ctrl++"));
    shortcuts.push_back(QKeySequence("Ctrl+="));
    ui.actionZoomIn->setShortcuts(shortcuts);

    // settings
    GtApplication *application = GtApplication::instance();
    GtMainSettings *settings = application->settings();
    QWidget *activeWindow = QApplication::activeWindow();

    if (activeWindow && !activeWindow->isMaximized()) {
        resize(QApplication::activeWindow()->size());
    }
    else {
        restoreGeometry(settings->geometry());
    }

    recentFiles = settings->recentFiles();
    lastOpenPath = settings->lastOpenPath();
    updateRecentFileActions();

    setCurrentFile("");
}

GtMainWindow::~GtMainWindow()
{
}

GtTabView* GtMainWindow::tabView(int index)
{
    if (-1 == index)
        index = ui.tabWidget->currentIndex();

    return qobject_cast<GtTabView*>(ui.tabWidget->widget(index));
}

GtDocTabView* GtMainWindow::newDocTab()
{
    GtDocTabView *tab = new GtDocTabView;
    openTab(tab);
    return tab;
}

void GtMainWindow::showTabContextMenu(const QPoint &pos)
{
    QMenu menu;
    QTabBar *tabBar = ui.tabWidget->tabBar();

    menu.addAction(ui.actionNewTab);
    int index = tabBar->tabAt(pos);
    if (index != -1) {
        menu.addSeparator();

        QAction *action = menu.addAction(tr("&Close Tab"),
                                         this,
                                         SLOT(on_actionCloseTab_triggered()),
                                         QKeySequence::Close);
        action->setData(index);


        action = menu.addAction(tr("Close &Other Tabs"),
                                this, SLOT(closeOtherTabs()));
        action->setData(index);
    }
    else {
    }

    menu.exec(QCursor::pos());
}

void GtMainWindow::closeTab(int index)
{
    QTabBar *tabBar = ui.tabWidget->tabBar();

    if (index < 0)
        index = tabBar->currentIndex();

    if (index < 0 || index >= tabBar->count())
        return;

    bool hasFocus = false;

    GtTabView *view = tabView(index);
    if (view) {
        hasFocus = view->hasFocus();
        view->deleteLater();
    }

    ui.tabWidget->removeTab(index);

    if (tabBar->count() < 2)
        tabBar->hide();

    if (hasFocus && tabBar->count() > 0)
        tabView()->setFocus();
}

void GtMainWindow::closeOtherTabs()
{
    int index = -1;
    QAction *action = qobject_cast<QAction*>(sender());

    if (action && !action->data().isNull())
        index = action->data().toInt();

    QTabBar *tabBar = ui.tabWidget->tabBar();

    if (index < 0)
        index = tabBar->currentIndex();

    if (index < 0 || index >= tabBar->count())
        return;

    // right tabs
    for (int i = tabBar->count() - 1; i > index; --i)
        closeTab(i);

    // left tabs
    for (int i = index - 1; i >= 0; --i)
        closeTab(i);
}

void GtMainWindow::on_actionNewWindow_triggered()
{
    GtMainWindow *mw = GtApplication::instance()->newMainWindow();
    (void)mw;
}

void GtMainWindow::on_actionNewTab_triggered()
{
    (void)newDocTab();
}

void GtMainWindow::on_actionOpenFile_triggered()
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

void GtMainWindow::on_actionCloseTab_triggered()
{
    int index = -1;
    QAction *action = qobject_cast<QAction*>(sender());

    if (action && !action->data().isNull())
        index = action->data().toInt();

    closeTab(index);
}

void GtMainWindow::on_actionQuit_triggered()
{
    close();
}

void GtMainWindow::on_actionCut_triggered()
{
    tabView()->onCut();
}

void GtMainWindow::on_actionCopy_triggered()
{
    tabView()->onCopy();
}

void GtMainWindow::on_actionPaste_triggered()
{
    tabView()->onPaste();
}

void GtMainWindow::on_actionDelete_triggered()
{
    tabView()->onDelete();
}

void GtMainWindow::on_actionZoomIn_triggered()
{
    tabView()->onZoomIn();
}

void GtMainWindow::on_actionZoomOut_triggered()
{
    tabView()->onZoomOut();
}

void GtMainWindow::on_actionRotateLeft_triggered()
{
    tabView()->onRotateLeft();
}

void GtMainWindow::on_actionRotateRight_triggered()
{
    tabView()->onRotateRight();
}

void GtMainWindow::on_actionAboutGather_triggered()
{
}

void GtMainWindow::openTab(GtTabView *tab)
{
    tab->m_ui = &ui;
    ui.tabWidget->addTab(tab, tr("(Untitled)"));
    ui.tabWidget->setCurrentWidget(tab);

    QTabBar *tabBar = ui.tabWidget->tabBar();
    if (tabBar->count() == 2)
        tabBar->show();
}

bool GtMainWindow::okToContinue()
{
    if (isWindowModified()) {
        int r = QMessageBox::warning(this, tr("hello"),
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

bool GtMainWindow::loadFile(const QString &fileName)
{
    GtDocumentPointer document;

    document = GtApplication::instance()->loadDocument(fileName);
    if (!document)
        return false;

    GtDocTabView *view = qobject_cast<GtDocTabView*>(tabView());
    if (!view)
        view = newDocTab();

    view->setDocument(document);
    setCurrentFile(fileName);
    statusBar()->showMessage(tr("File loaded"), 2000);
    return true;
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

    recentFileSeparator->setVisible(!recentFiles.isEmpty());
}

QString GtMainWindow::strippedName(const QString &fullFileName)
{
    return QFileInfo(fullFileName).fileName();
}

void GtMainWindow::openRecentFile()
{
    if (okToContinue()) {
        QAction *action = qobject_cast<QAction *>(sender());
        if (action)
            loadFile(action->data().toString());
    }
}

void GtMainWindow::changeEvent(QEvent *event)
{
    QWidget::changeEvent(event);

    switch (event->type()) {
    case QEvent::LanguageChange:
        ui.retranslateUi(this);
        break;

    default:
        break;
    }
}

void GtMainWindow::closeEvent(QCloseEvent *event)
{
    if (okToContinue()) {
        GtApplication *application = GtApplication::instance();
        if (application->mainWindows().size() == 1) {
            GtMainSettings *settings = application->settings();
            settings->setGeometry(saveGeometry());
            settings->setRecentFiles(recentFiles);
            settings->setLastOpenPath(lastOpenPath);

            // notify opening tabs of close event
            GtTabView *current = tabView();
            for (int i = 0; i < ui.tabWidget->count(); ++i) {
                GtTabView *tab = tabView(i);
                tab->mainWindowClose(current);
            }
        }

        event->accept();
    }
    else {
        event->ignore();
    }
}

GT_END_NAMESPACE
