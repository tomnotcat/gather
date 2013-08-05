/*
 * Copyright (C) 2013 Tom Wong. All rights reserved.
 */
#include "gtdoctabview.h"
#include "gtapplication.h"
#include "gtbookmark.h"
#include "gtbookmarks.h"
#include "gtdocmodel.h"
#include "gtdocpage.h"
#include "gtdocrange.h"
#include "gtdocument.h"
#include "gtdocview.h"
#include "gtmainsettings.h"
#include "gtmainwindow.h"
#include "gttocdelegate.h"
#include "gttocmodel.h"
#include <QtCore/QDebug>
#include <QtWidgets/QMenu>
#include <QtWidgets/QSplitter>
#include <QtWidgets/QTreeView>
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
    // model
    m_tocModel = new GtTocModel(this);

    // view
    m_verticalLayout = new QVBoxLayout(this);
    m_verticalLayout->setObjectName(QStringLiteral("verticalLayout"));
    m_verticalLayout->setMargin(0);

    m_splitter = new QSplitter(this);
    m_splitter->setObjectName(QStringLiteral("splitter"));
    m_splitter->setOrientation(Qt::Horizontal);

    // tab of contents
    m_tocView = new QTreeView(m_splitter);
    m_tocView->setObjectName(QStringLiteral("tocView"));
    m_tocView->setFrameShape(QFrame::NoFrame);
    m_tocView->setHeaderHidden(true);
    m_tocView->setItemDelegate(new GtTocDelegate(m_tocView));
    m_splitter->addWidget(m_tocView);

    connect(m_tocView, SIGNAL(clicked(QModelIndex)),
            this, SLOT(tocChanged(QModelIndex)));
    connect(m_tocView, SIGNAL(activated(QModelIndex)),
            this, SLOT(tocChanged(QModelIndex)));

    // document view
    m_docView = new GtDocView(m_splitter);
    m_docView->setMinimumWidth(120);
    m_docView->setObjectName(QStringLiteral("docView"));
    m_docView->setRenderThread(GtApplication::instance()->docThread());
    m_docView->setRenderCacheSize(1024 * 1024 * 20);
    m_docView->setContextMenuPolicy(Qt::CustomContextMenu);
    m_splitter->addWidget(m_docView);
    m_splitter->setStretchFactor(0, 0);
    m_splitter->setStretchFactor(1, 255);

    connect(m_docView, SIGNAL(customContextMenuRequested(const QPoint&)),
            this, SLOT(showDocViewContextMenu(const QPoint&)));

    m_verticalLayout->addWidget(m_splitter);

    // settings
    GtApplication *application = GtApplication::instance();
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

void GtDocTabView::showDocViewContextMenu(const QPoint &pos)
{
    GtDocRange selRange(m_docView->selectedRange());
    Ui_MainWindow &ui = mainWindow()->m_ui;
    QMenu menu;

    if (selRange.isEmpty()) {
        menu.addAction(tr("S&elect Tool"), m_docView, SLOT(highlight()));
        menu.addAction(tr("Ha&nd Tool"), m_docView, SLOT(highlight()));
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

void GtDocTabView::tocChanged(const QModelIndex &index)
{
    GtBookmark *bookmark = m_tocModel->bookmarkFromIndex(index);
    if (bookmark) {
        m_docView->scrollTo(bookmark->dest());
    }
}

void GtDocTabView::addBookmark()
{
    GtBookmarks *bookmarks = m_docModel->bookmarks();
    GtBookmark *bookmark = new GtBookmark(m_docView->scrollDest());

    QModelIndex index = m_tocView->currentIndex();
    GtBookmark *prev = m_tocModel->bookmarkFromIndex(index);
    GtBookmark *parent = bookmarks->root();

    if (prev)
        parent = prev->parent();
    else
        parent = bookmarks->root();

    parent->insert(prev ? prev->next() : 0, bookmark);
    emit bookmarks->added(bookmark);
}

void GtDocTabView::searchSelectedText()
{
    qDebug() << "search selected text";
}

GT_END_NAMESPACE
