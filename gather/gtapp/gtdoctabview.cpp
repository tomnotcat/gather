/*
 * Copyright (C) 2013 Tom Wong. All rights reserved.
 */
#include "gtdoctabview.h"
#include "gtapplication.h"
#include "gtbookmark.h"
#include "gtbookmarks.h"
#include "gtdoccommand.h"
#include "gtdocmodel.h"
#include "gtdocpage.h"
#include "gtdocrange.h"
#include "gtdocument.h"
#include "gtdocview.h"
#include "gtmainsettings.h"
#include "gtmainwindow.h"
#include "gttocmodel.h"
#include "gttocview.h"
#include <QtCore/QDebug>
#include <QtWidgets/QMenu>
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QShortcut>
#include <QtWidgets/QSplitter>
#include <QtWidgets/QUndoStack>
#include <QtWidgets/QVBoxLayout>

GT_BEGIN_NAMESPACE

GtDocTabView::GtDocTabView(QWidget *parent)
    : GtTabView(parent)
    , m_docModel(0)
    , m_undoStack(0)
    , m_undoAction(0)
    , m_redoAction(0)
{
    GtApplication *application = GtApplication::instance();

    // view
    m_verticalLayout = new QVBoxLayout(this);
    m_verticalLayout->setObjectName(QStringLiteral("verticalLayout"));
    m_verticalLayout->setMargin(0);

    m_splitter = new QSplitter(this);
    m_splitter->setObjectName(QStringLiteral("splitter"));
    m_splitter->setOrientation(Qt::Horizontal);

    // tab of contents
    m_tocModel = new GtTocModel(this);
    m_tocView = new GtTocView(m_splitter);
    m_tocView->setObjectName(QStringLiteral("tocView"));

    m_splitter->addWidget(m_tocView);

    connect(m_tocView, SIGNAL(gotoBookmark(GtBookmark*)),
            this, SLOT(gotoBookmark(GtBookmark*)));
    connect(m_tocView, SIGNAL(customContextMenuRequested(const QPoint&)),
            this, SLOT(tocViewContextMenu(const QPoint&)));

    // document view
    m_docView = new GtDocView(application->docThread(), m_splitter);
    m_docView->setMinimumWidth(120);
    m_docView->setObjectName(QStringLiteral("docView"));
    m_docView->setRenderCacheSize(1024 * 1024 * 20);
    m_docView->setContextMenuPolicy(Qt::CustomContextMenu);
    m_splitter->addWidget(m_docView);
    m_splitter->setStretchFactor(0, 0);
    m_splitter->setStretchFactor(1, 255);

    connect(m_docView, SIGNAL(customContextMenuRequested(const QPoint&)),
            this, SLOT(docViewContextMenu(const QPoint&)));

    m_verticalLayout->addWidget(m_splitter);

    QShortcut *shortcut = new QShortcut(QKeySequence::Delete, this);
    connect(shortcut, SIGNAL(activated()), this, SLOT(onDelete()));

    // settings
    GtMainSettings *settings = application->settings();
    m_splitter->restoreState(settings->docSplitter());
}

GtDocTabView::~GtDocTabView()
{
    m_docView->setModel(0);
    m_tocView->setModel(0);
    m_tocModel->setDocModel(0);

    if (m_docModel)
        m_docModel->release();
}

void GtDocTabView::setDocModel(GtDocModel *docModel)
{
    m_tocModel->setDocModel(0);
    m_tocView->setModel(0);

    // release old model first
    if (m_docModel) {
        disconnect(m_docModel->document(),
                   SIGNAL(loaded(GtDocument*)),
                   this,
                   SLOT(documentLoaded(GtDocument*)));
        m_docModel->release();
        m_docModel = 0;
    }

    m_docModel = docModel;
    m_docView->setModel(docModel);

    if (0 == m_docModel)
        return;

    m_docModel->ref.ref();
    if (!m_docModel->document()->isLoaded()) {
        connect(m_docModel->document(), SIGNAL(loaded(GtDocument*)),
                this, SLOT(documentLoaded(GtDocument*)));
    }

    if (m_docModel->document()->isLoaded())
        documentLoaded(m_docModel->document());
}

void GtDocTabView::setUndoStack(QUndoStack *undoStack)
{
    m_undoStack = undoStack;
    m_docView->setUndoStack(undoStack);
    m_tocModel->setUndoStack(undoStack);
    m_tocView->setUndoStack(undoStack);

    QAction *undoAction = m_undoStack->createUndoAction(this);
    undoAction->setShortcuts(QKeySequence::Undo);

    QAction *redoAction = m_undoStack->createRedoAction(this);
    redoAction->setShortcuts(QKeySequence::Redo);

    if (isActive()) {
        Ui_MainWindow &ui = mainWindow()->m_ui;

        if (m_undoAction && m_redoAction) {
            ui.menuEdit->insertAction(m_undoAction, redoAction);
            ui.menuEdit->insertAction(redoAction, undoAction);

            ui.menuEdit->removeAction(m_undoAction);
            ui.menuEdit->removeAction(m_redoAction);
        }
        else {
            ui.menuEdit->insertAction(ui.actionUndo, redoAction);
            ui.menuEdit->insertAction(redoAction, undoAction);

            ui.menuEdit->removeAction(ui.actionUndo);
            ui.menuEdit->removeAction(ui.actionRedo);
        }
    }

    delete m_undoAction;
    delete m_redoAction;
    m_undoAction = undoAction;
    m_redoAction = redoAction;
}

void GtDocTabView::gainActive()
{
    Ui_MainWindow &ui = mainWindow()->m_ui;

    if (m_undoAction && m_redoAction)  {
        ui.menuEdit->insertAction(ui.actionUndo, m_redoAction);
        ui.menuEdit->insertAction(m_redoAction, m_undoAction);

        ui.menuEdit->removeAction(ui.actionUndo);
        ui.menuEdit->removeAction(ui.actionRedo);
    }

    ui.actionCut->setEnabled(true);
    ui.actionCopy->setEnabled(true);
    ui.actionPaste->setEnabled(true);
    ui.actionDelete->setEnabled(true);
    ui.actionAddBookmark->setEnabled(true);

    ui.actionRotateLeft->setEnabled(true);
    ui.actionRotateRight->setEnabled(true);
    ui.actionZoomIn->setEnabled(true);
    ui.actionZoomOut->setEnabled(true);
    ui.actionZoomTo->setEnabled(true);

    connect(ui.actionCopy, SIGNAL(triggered()),
            m_docView, SLOT(copy()));
    connect(ui.actionDelete, SIGNAL(triggered()),
            this, SLOT(onDelete()));
    connect(ui.actionAddBookmark, SIGNAL(triggered()),
            this, SLOT(addBookmark()));

    connect(ui.actionRotateLeft, SIGNAL(triggered()),
            m_docView, SLOT(rotateLeft()));
    connect(ui.actionRotateRight, SIGNAL(triggered()),
            m_docView, SLOT(rotateRight()));
    connect(ui.actionZoomIn, SIGNAL(triggered()),
            m_docView, SLOT(zoomIn()));
    connect(ui.actionZoomOut, SIGNAL(triggered()),
            m_docView, SLOT(zoomOut()));

    GtTabView::gainActive();
}

void GtDocTabView::loseActive()
{
    GtTabView::loseActive();

    Ui_MainWindow &ui = mainWindow()->m_ui;

    if (m_undoAction && m_redoAction)  {
        ui.menuEdit->insertAction(m_undoAction, ui.actionRedo);
        ui.menuEdit->insertAction(ui.actionRedo, ui.actionUndo);

        ui.menuEdit->removeAction(m_undoAction);
        ui.menuEdit->removeAction(m_redoAction);
    }

    ui.actionCut->setEnabled(false);
    ui.actionCopy->setEnabled(false);
    ui.actionPaste->setEnabled(false);
    ui.actionDelete->setEnabled(false);
    ui.actionAddBookmark->setEnabled(false);

    ui.actionRotateLeft->setEnabled(false);
    ui.actionRotateRight->setEnabled(false);
    ui.actionZoomIn->setEnabled(false);
    ui.actionZoomOut->setEnabled(false);
    ui.actionZoomTo->setEnabled(false);

    disconnect(ui.actionCopy, SIGNAL(triggered()),
               m_docView, SLOT(copy()));
    disconnect(ui.actionDelete, SIGNAL(triggered()),
               this, SLOT(onDelete()));
    disconnect(ui.actionAddBookmark, SIGNAL(triggered()),
               this, SLOT(addBookmark()));

    disconnect(ui.actionRotateLeft, SIGNAL(triggered()),
               m_docView, SLOT(rotateLeft()));
    disconnect(ui.actionRotateRight, SIGNAL(triggered()),
               m_docView, SLOT(rotateRight()));
    disconnect(ui.actionZoomIn, SIGNAL(triggered()),
               m_docView, SLOT(zoomIn()));
    disconnect(ui.actionZoomOut, SIGNAL(triggered()),
               m_docView, SLOT(zoomOut()));
}

void GtDocTabView::saveSettings(GtMainSettings *settings)
{
    if (mainWindow()->tabView() != this)
        return;

    settings->setDocSplitter(m_splitter->saveState());
}

void GtDocTabView::onDelete()
{
    if (m_docView->hasFocus()) {
        return;
    }

    if (m_tocView->hasFocus()) {
        m_tocView->deleteBookmark();
        return;
    }
}

void GtDocTabView::docViewContextMenu(const QPoint &pos)
{
    GtDocRange selRange(m_docView->selectedRange());
    Ui_MainWindow &ui = mainWindow()->m_ui;
    QMenu menu;

    if (selRange.isEmpty()) {
        menu.addAction(tr("S&elect Tool"), m_docView, SLOT(highlight()));
        menu.addAction(tr("Ha&nd Tool"), m_docView, SLOT(highlight()));
        menu.addSeparator();
        menu.addAction(ui.actionAddBookmark);
    }
    else {
        menu.addAction(ui.actionCopy);
        menu.addSeparator();
        menu.addAction(tr("&Highlight"),
                       m_docView, SLOT(highlight()));
        menu.addAction(tr("&Underline"),
                       m_docView, SLOT(underline()));
        menu.addSeparator();
        menu.addAction(ui.actionAddBookmark);
        menu.addSeparator();
        menu.addAction(tr("&Search"),
                       this, SLOT(searchSelectedText()));
    }

    menu.exec(m_docView->mapToGlobal(pos));
}

void GtDocTabView::tocViewContextMenu(const QPoint &pos)
{
    QModelIndex index = m_tocView->indexAt(pos);

    if (!index.isValid())
        return;

    Ui_MainWindow &ui = mainWindow()->m_ui;
    QMenu menu;

    menu.addAction(tr("&Goto Bookmark"), this, SLOT(gotoBookmark()));
    menu.addSeparator();
    menu.addAction(ui.actionAddBookmark);
    menu.addAction(tr("&Set Destination"), this, SLOT(setDestination()));
    menu.addSeparator();
    menu.addAction(ui.actionDelete);
    menu.addAction(tr("&Rename"), m_tocView, SLOT(renameBookmark()));
    menu.exec(m_tocView->mapToGlobal(pos));
}

void GtDocTabView::documentLoaded(GtDocument *document)
{
    if (document != m_docModel->document()) {
        qWarning() << "invalid document loaded:"
                   << m_docModel->document() << document;
        return;
    }

    if (document->isLoaded()) {
        m_tocModel->setDocModel(m_docModel);
        m_tocView->setModel(m_tocModel);
        m_docView->setFocus();

        // set tab text with document title
        QTabWidget *tabWidget = mainWindow()->m_ui.tabWidget;
        int index = tabWidget->indexOf(this);
        tabWidget->setTabText(index, document->title());
        tabWidget->setTabToolTip(index, document->title());

        mainWindow()->setWindowTitle(document->title());
    }
    else {
        Q_ASSERT(0);
    }
}

void GtDocTabView::gotoBookmark(GtBookmark *bookmark)
{
    if (!bookmark)
        bookmark = m_tocView->currentBookmark();

    if (bookmark)
        m_docView->scrollTo(bookmark->dest());
}

void GtDocTabView::addBookmark()
{
    QModelIndex index = m_tocView->currentIndex();
    GtBookmark *current = m_tocModel->bookmarkFromIndex(index);
    GtBookmark *bookmark = new GtBookmark(m_docView->scrollDest());

    QString title(m_docView->selectedText());
    if (!title.isEmpty()) {
        title.replace(QChar::LineFeed, QChar::Space);
        title.truncate(MaxBookmarkTitle);
        bookmark->setTitle(title);
    }

    QUndoCommand *command = new GtAddBookmarkCommand(
        m_docModel,
        current ? current->parent() : 0,
        current ? current->next() : 0,
        bookmark);

    m_undoStack->push(command);

    // edit the added bookmark
    index = m_tocModel->indexFromBookmark(bookmark);
    m_tocView->setCurrentIndex(index);
    m_tocView->edit(index);
}

void GtDocTabView::setDestination()
{
    GtBookmark *bookmark = m_tocView->currentBookmark();
    if (!bookmark)
        return;

    QMessageBox confirm(QMessageBox::Question,
                        tr("Gather Reader"),
                        tr("Set the bookmark destination to current viewport ?"),
                        QMessageBox::Yes | QMessageBox::No,
                        this);
    confirm.setDefaultButton(QMessageBox::Yes);

    if (confirm.exec() == QMessageBox::No)
        return;

    GtLinkDest dest(m_docView->scrollDest());
    bookmark->setDest(dest);

    GtBookmarks *bookmarks = m_docModel->bookmarks();
    emit bookmarks->updated(bookmark, GtBookmark::UpdateDest);
}

void GtDocTabView::searchSelectedText()
{
    qDebug() << "search selected text";
}

GT_END_NAMESPACE
