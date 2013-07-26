/*
 * Copyright (C) 2013 Tom Wong. All rights reserved.
 */
#include "gtdoctabview.h"
#include "gtapplication.h"
#include "gtbookmark.h"
#include "gtdocpage.h"
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
#include <QtWidgets/QVBoxLayout>

GT_BEGIN_NAMESPACE

GtDocTabView::GtDocTabView(QWidget *parent)
    : GtTabView(parent)
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
}

GtDocModelPointer GtDocTabView::docModel() const
{
    return m_docModel;
}

void GtDocTabView::setDocModel(GtDocModelPointer docModel)
{
    m_tocModel->setDocModel(0);
    m_tocView->setModel(0);

    // release old model first
    if (m_docModel) {
        disconnect(m_docModel->document(),
                   SIGNAL(loaded(GtDocument*)),
                   this,
                   SLOT(documentLoaded(GtDocument*)));
        m_docModel = 0;
    }

    m_docModel = docModel;
    m_docView->setModel(docModel.data());

    if (0 == m_docModel)
        return;

    if (m_docModel->document()->isLoaded()) {
        documentLoaded(m_docModel->document());
    }
    else {
        connect(m_docModel->document(), SIGNAL(loaded(GtDocument*)),
                this, SLOT(documentLoaded(GtDocument*)));
    }
}

void GtDocTabView::onCut()
{
}

void GtDocTabView::onCopy()
{
    m_docView->copy();
}

void GtDocTabView::onPaste()
{
}

void GtDocTabView::onDelete()
{
}

void GtDocTabView::onZoomIn()
{
    m_docView->zoomIn();
}

void GtDocTabView::onZoomOut()
{
    m_docView->zoomOut();
}

void GtDocTabView::onRotateLeft()
{
    m_docModel->setRotation(m_docModel->rotation() - 90);
}

void GtDocTabView::onRotateRight()
{
    m_docModel->setRotation(m_docModel->rotation() + 90);
}

void GtDocTabView::currentChanged(GtTabView *old, GtTabView *now)
{
}

void GtDocTabView::mainWindowClose(GtTabView *current)
{
    if (this != current)
        return;

    GtApplication *application = GtApplication::instance();
    GtMainSettings *settings = application->settings();
    settings->setDocSplitter(m_splitter->saveState());
}

void GtDocTabView::showDocViewContextMenu(const QPoint &pos)
{
    GtDocRange selRange(m_docView->selectedRange());
    QMenu menu;

    if (selRange.isEmpty()) {
        menu.addAction(tr("S&elect Tool"), m_docView, SLOT(highlight()));
        menu.addAction(tr("Ha&nd Tool"), m_docView, SLOT(highlight()));
    }
    else {
        menu.addAction(m_mainWindow->ui.actionCopy);
        menu.addSeparator();
        menu.addAction(tr("&Highlight Text"), m_docView, SLOT(highlight()));
        menu.addAction(tr("&Add Bookmark"), this, SLOT(addBookmark()));
        menu.addSeparator();
        menu.addAction(tr("&Search"), this, SLOT(searchSelectedText()));
    }

    menu.exec(QCursor::pos());
}

void GtDocTabView::documentLoaded(GtDocument *document)
{
    if (document != m_docModel->document()) {
        qWarning() << "invalid document loaded:"
                   << m_docModel->document() << document;
        return;
    }

    if (document->isLoaded()) {
        m_tocModel->setDocModel(m_docModel.data());
        m_tocView->setModel(m_tocModel);
        m_docView->setFocus();

        // set tab text with document title
        QTabWidget *tabWidget = m_mainWindow->ui.tabWidget;
        int index = tabWidget->indexOf(this);
        tabWidget->setTabText(index, document->title());
        tabWidget->setTabToolTip(index, document->title());

        m_mainWindow->setWindowTitle(document->title());
    }
    else {
        Q_ASSERT(0);
    }
}

void GtDocTabView::tocChanged(const QModelIndex &index)
{
    GtBookmark *bookmark = m_tocModel->bookmarkFromIndex(index);
    if (bookmark) {
        // TODO: do not scroll when the page is already in viewport
        QRect rect(m_docView->pageExtents(bookmark->dest().page()));
        m_docView->scrollTo(rect.x(), rect.y());
    }
}

void GtDocTabView::addBookmark()
{
    qDebug() << "add bookmark";
}

void GtDocTabView::searchSelectedText()
{
    qDebug() << "search selected text";
}

GT_END_NAMESPACE
