/*
 * Copyright (C) 2013 Tom Wong. All rights reserved.
 */
#include "gtdoctabview.h"
#include "gtapplication.h"
#include "gtdocmodel.h"
#include "gtdocoutline.h"
#include "gtdocpage.h"
#include "gtdocview.h"
#include "gtmainsettings.h"
#include "gttocdelegate.h"
#include "gttocmodel.h"
#include "ui_gtmainwindow.h"
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
    m_docModel = new GtDocModel(this);
    m_docModel->setMinScale(0.1);
    m_docModel->setMaxScale(4.0);
    m_docModel->setMouseMode(GtDocModel::SelectText);

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
    m_tocView->setFrameStyle(QFrame::NoFrame);
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
    m_docView->setModel(m_docModel);
    m_docView->setRenderThread(GtApplication::instance()->renderThread());
    m_docView->setRenderCacheSize(1024 * 1024 * 20);
    m_docView->setContextMenuPolicy(Qt::CustomContextMenu);
    m_splitter->addWidget(m_docView);
    m_splitter->setStretchFactor(0, 0);
    m_splitter->setStretchFactor(1, 255);

    connect(m_docView,
            SIGNAL(customContextMenuRequested(const QPoint&)),
            this,
            SLOT(showDocViewContextMenu(const QPoint&)));

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
    m_docModel->setDocument(0);
    m_tocModel->setDocument(0);
}

GtDocumentPointer GtDocTabView::document() const
{
    return m_document;
}

void GtDocTabView::setDocument(GtDocumentPointer document)
{
    m_docModel->setDocument(0);
    m_tocModel->setDocument(0);
    m_tocView->setModel(0);

    // release old document first
    if (m_document) {
        disconnect(m_document.data(),
                   SIGNAL(loaded(GtDocument*)),
                   this,
                   SLOT(docLoaded(GtDocument*)));
        m_document = 0;
    }

    m_document = document;
    if (0 == m_document)
        return;

    if (m_document->isLoaded()) {
        docLoaded(m_document.data());
    }
    else {
        connect(m_document.data(), SIGNAL(loaded(GtDocument*)),
                this, SLOT(docLoaded(GtDocument*)));
    }
}

void GtDocTabView::onCut()
{
}

void GtDocTabView::onCopy()
{
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
    GtDocRange selRange(m_docView->selectRange());

    if (selRange.isEmpty())
        return;

    QMenu *menu = new QMenu(m_docView);

    QAction *searchAction = new QAction("&Search", menu);
    connect(searchAction, SIGNAL(triggered()),
            this, SLOT(searchSelectedText()));

    menu->addAction(m_ui->actionCopy);
    menu->addSeparator();
    menu->addAction(searchAction);

    menu->exec(QCursor::pos());
}

void GtDocTabView::docLoaded(GtDocument *doc)
{
    if (doc != m_document.data()) {
        qWarning() << "invalid document loaded:" << m_document.data() << doc;
        return;
    }

    if (m_document->isLoaded()) {
        m_docModel->setDocument(m_document.data());
        m_tocModel->setDocument(m_document.data());
        m_tocView->setModel(m_tocModel);
    }
    else {
        Q_ASSERT(0);
    }
}

void GtDocTabView::tocChanged(const QModelIndex &index)
{
    GtDocOutline *outline = m_tocModel->outlineFromIndex(index);
    if (outline) {
        // TODO: do not scroll when the page is already in viewport
        QRect rect(m_docView->pageExtents(outline->page));
        m_docView->scrollTo(rect.x(), rect.y());
    }
}

void GtDocTabView::searchSelectedText()
{
}

GT_END_NAMESPACE
