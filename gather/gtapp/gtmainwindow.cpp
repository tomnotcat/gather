/*
 * Copyright (C) 2013 Tom Wong. All rights reserved.
 */
#include "gtmainwindow.h"
#include "gtapplication.h"
#include "gtdocmanager.h"
#include "gtdoctabview.h"
#include "gthometabview.h"
#include "gtlogindialog.h"
#include "gtmainsettings.h"
#include <QtCore/QtDebug>
#include <QtGui/QCloseEvent>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QMessageBox>

GT_BEGIN_NAMESPACE

GtMainWindow::GtMainWindow()
    : m_oldTabView(0)
{
    m_ui.setupUi(this);

    setToolButtonStyle(Qt::ToolButtonFollowStyle);
    setAttribute(Qt::WA_DeleteOnClose, true);

    // tab widget
    QTabBar *tabBar = m_ui.tabWidget->tabBar();

    tabBar->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(tabBar, SIGNAL(customContextMenuRequested(const QPoint&)),
            this, SLOT(showTabContextMenu(const QPoint&)));
    connect(m_ui.tabWidget, SIGNAL(tabCloseRequested(int)),
            this, SLOT(closeTab(int)));
    tabBar->setSelectionBehaviorOnRemove(QTabBar::SelectPreviousTab);
    tabBar->hide();

    connect(m_ui.tabWidget, SIGNAL(currentChanged(int)),
            this, SLOT(currentTabChanged(int)));

    // recent files
    m_recentFileActions[0] = m_ui.actionRecentFile0;
    m_recentFileActions[1] = m_ui.actionRecentFile1;
    m_recentFileActions[2] = m_ui.actionRecentFile2;
    m_recentFileActions[3] = m_ui.actionRecentFile3;
    m_recentFileActions[4] = m_ui.actionRecentFile4;

    for (int i = 0; i < MaxRecentFiles; ++i) {
        m_recentFileActions[i]->setVisible(false);
        connect(m_recentFileActions[i], SIGNAL(triggered()),
                this, SLOT(openRecentFile()));
    }

    m_recentFileSeparator = m_ui.menuFile->insertSeparator(m_ui.actionCloseTab);

    // menu and shortcuts
    connect(m_ui.menuEdit, SIGNAL(aboutToShow()),
            this, SLOT(editMenuAboutToShow()));
    connect(m_ui.menuEdit, SIGNAL(aboutToHide()),
            this, SLOT(editMenuAboutToHide()));

    QList<QKeySequence> shortcuts;
    shortcuts.push_back(QKeySequence("Ctrl++"));
    shortcuts.push_back(QKeySequence("Ctrl+="));
    m_ui.actionZoomIn->setShortcuts(shortcuts);

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

    m_recentFiles = settings->recentFiles();
    m_lastOpenPath = settings->lastOpenPath();
    updateRecentFileActions();

    setCurrentFile("");

    (void)newTab();
}

GtMainWindow::~GtMainWindow()
{
}

GtTabView* GtMainWindow::tabView(int index)
{
    if (-1 == index)
        index = m_ui.tabWidget->currentIndex();

    return qobject_cast<GtTabView*>(m_ui.tabWidget->widget(index));
}

GtTabView* GtMainWindow::newTab()
{
    GtHomeTabView *tab = new GtHomeTabView(this);
    openTab(tab);
    return tab;
}

void GtMainWindow::showTabContextMenu(const QPoint &pos)
{
    QMenu menu;
    QTabBar *tabBar = m_ui.tabWidget->tabBar();

    menu.addAction(m_ui.actionNewTab);
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
    QTabBar *tabBar = m_ui.tabWidget->tabBar();

    if (index < 0)
        index = tabBar->currentIndex();

    if (index < 0 || index >= tabBar->count())
        return;

    bool hasFocus = false;

    GtTabView *view = tabView(index);
    if (view) {
        if (view == m_oldTabView) {
            m_oldTabView->loseActive();
            m_oldTabView = 0;
        }

        hasFocus = view->hasFocus();
        view->deleteLater();
    }

    m_ui.tabWidget->removeTab(index);

    if (tabBar->count() < 2)
        tabBar->hide();

    if (tabBar->count() == 0) {
#if defined(Q_WS_MAC)
        close();
#else
        (void)newTab();
#endif
    }
    else if (hasFocus) {
        tabView()->setFocus();
    }
}

void GtMainWindow::closeOtherTabs()
{
    int index = -1;
    QAction *action = qobject_cast<QAction*>(sender());

    if (action && !action->data().isNull())
        index = action->data().toInt();

    QTabBar *tabBar = m_ui.tabWidget->tabBar();

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

void GtMainWindow::on_actionNewTab_triggered()
{
    (void)newTab();
}

void GtMainWindow::on_actionNewWindow_triggered()
{
    (void)GtApplication::instance()->newMainWindow();
}

void GtMainWindow::on_actionOpenFile_triggered()
{
    if (okToContinue()) {
        QString fileName = QFileDialog::getOpenFileName(
            this, tr("Open Document"), m_lastOpenPath,
            tr("Document Files (*.pdf *.txt);;All Files (*.*)"));

        if (fileName.isEmpty())
            return;

        if (loadFile(fileName))
            m_lastOpenPath = QFileInfo(fileName).path();
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

void GtMainWindow::on_actionLogin_triggered()
{
    GtLoginDialog dialog(this);
    dialog.exec();
}

void GtMainWindow::on_actionAboutGather_triggered()
{
}

void GtMainWindow::openTab(GtTabView *tab)
{
    int index = m_ui.tabWidget->addTab(tab, tr("New Tab"));
    m_ui.tabWidget->setTabToolTip(index, tr("New Tab"));
    m_ui.tabWidget->setCurrentWidget(tab);

    QTabBar *tabBar = m_ui.tabWidget->tabBar();
    if (tabBar->count() == 2)
        tabBar->show();
}

bool GtMainWindow::okToContinue()
{
    return true;
}

bool GtMainWindow::loadFile(const QString &fileName)
{
    GtDocManager *docManager = GtApplication::instance()->docManager();
    GtDocModel *docModel = docManager->loadLocalDocument(fileName);

    if (!docModel) {
        QMessageBox::warning(this, tr("Gather Reader"),
                             tr("The document could not be loaded."),
                             QMessageBox::Cancel);
        return false;
    }

    GtTabView *view = tabView();
    GtDocTabView *docView = qobject_cast<GtDocTabView*>(tabView());
    if (!docView) {
        view->deleteLater();
        docView = new GtDocTabView(this);
        view = docView;

        int index = m_ui.tabWidget->currentIndex();
        m_ui.tabWidget->removeTab(index);
        m_ui.tabWidget->insertTab(index, view, tr("New Tab"));
        m_ui.tabWidget->setCurrentWidget(view);
    }

    docView->setDocModel(docModel);

    QUndoStack *undoStack = docManager->undoStack(docModel);
    docView->setUndoStack(undoStack);

    setCurrentFile(fileName);
    statusBar()->showMessage(tr("File loaded"), 2000);
    return true;
}

void GtMainWindow::setCurrentFile(const QString &fileName)
{
    m_curFile = fileName;

    if (!m_curFile.isEmpty()) {
        m_recentFiles.removeAll(m_curFile);
        m_recentFiles.prepend(m_curFile);
        updateRecentFileActions();
    }
}

void GtMainWindow::updateRecentFileActions()
{
    QMutableStringListIterator it(m_recentFiles);
    while (it.hasNext()) {
        if (!QFile::exists(it.next()))
            it.remove();
    }

    for (int i = 0; i < MaxRecentFiles; ++i) {
        if (i < m_recentFiles.count()) {
            QString text = tr("&%1 %2")
                           .arg(i + 1)
                           .arg(strippedName(m_recentFiles[i]));
            m_recentFileActions[i]->setText(text);
            m_recentFileActions[i]->setData(m_recentFiles[i]);
            m_recentFileActions[i]->setVisible(true);
        } else {
            m_recentFileActions[i]->setVisible(false);
        }
    }

    m_recentFileSeparator->setVisible(!m_recentFiles.isEmpty());
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

void GtMainWindow::currentTabChanged(int index)
{
    if (m_oldTabView) {
        if (tabView(index) == m_oldTabView)
            return;

        m_oldTabView->loseActive();
        m_oldTabView = 0;
    }

    if (index != -1) {
        m_oldTabView = tabView(index);
        m_oldTabView->gainActive();
        setWindowTitle(m_ui.tabWidget->tabText(index));
    }
}

void GtMainWindow::editMenuAboutToShow()
{
    qDebug() << "show edit menu";
}

void GtMainWindow::editMenuAboutToHide()
{
    qDebug() << "hide edit menu";
}

void GtMainWindow::changeEvent(QEvent *event)
{
    QWidget::changeEvent(event);

    switch (event->type()) {
    case QEvent::LanguageChange:
        m_ui.retranslateUi(this);
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
            settings->setRecentFiles(m_recentFiles);
            settings->setLastOpenPath(m_lastOpenPath);

            // save opening tabs's settings
            for (int i = 0; i < m_ui.tabWidget->count(); ++i) {
                tabView(i)->saveSettings(settings);
            }
        }

        event->accept();
    }
    else {
        event->ignore();
    }
}

GT_END_NAMESPACE
