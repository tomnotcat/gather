/*
 * Copyright (C) 2013 Tom Wong. All rights reserved.
 */
#include "gtdocview.h"
#include "gtdocmodel.h"
#include "gtdocument.h"
#include <QtCore/QDebug>
#include <QtGui/QPaintEvent>
#include <QtWidgets/QScrollBar>

GT_BEGIN_NAMESPACE

class GtDocViewPrivate
{
    Q_DECLARE_PUBLIC(GtDocView)

public:
    GtDocViewPrivate(GtDocView *parent);
    ~GtDocViewPrivate();

public:
    void setupCaches();
    void clearCaches();
    void changePage(int page);
    void resizeContentArea(int width, int height);
    void updatePageStep();
    int getPageYOffset(int page);
    QSize getMaxPageSize();
    QRect computeBorder(const QSize &size);
    QSize layoutPagesContinuous();
    QSize layoutPagesContinuousDualPage();
    QSize layoutPages();
    QSize layoutPagesDualPage();

public:
    enum PendingScroll {
        SCROLL_TO_KEEP_POSITION,
        SCROLL_TO_PAGE_POSITION,
        SCROLL_TO_CENTER,
        SCROLL_TO_FIND_LOCATION
    };

private:
    GtDocView *q_ptr;
    GtDocModel *model;
    GtDocument *document;
    int beginPage;
    int endPage;
    int currentPage;
    int pageCount;
    int rotation;
    int spacing;
    double scale;
    bool continuous;
    GtDocModel::LayoutMode layoutMode;
    GtDocModel::SizingMode sizingMode;
    PendingScroll pendingScroll;
};

GtDocViewPrivate::GtDocViewPrivate(GtDocView *parent)
    : q_ptr(parent)
    , model(NULL)
    , document(NULL)
    , beginPage(-1)
    , endPage(-1)
    , currentPage(-1)
    , pageCount(0)
    , rotation(0)
    , spacing(5)
    , scale(1.0)
    , continuous(true)
    , layoutMode(GtDocModel::SinglePage)
    , sizingMode(GtDocModel::FitWidth)
    , pendingScroll(SCROLL_TO_KEEP_POSITION)
{
    Q_Q(GtDocView);

    q->setFrameStyle(QFrame::NoFrame);
    q->setAttribute(Qt::WA_StaticContents);

    // viewport setup: setup focus, accept drops and track mouse
    QWidget *vp = q->viewport();
    vp->setFocusProxy(q);
    vp->setFocusPolicy(Qt::StrongFocus);
    vp->setAttribute(Qt::WA_OpaquePaintEvent);
    vp->setAttribute(Qt::WA_NoSystemBackground);

    q->setAcceptDrops(true);
    vp->setMouseTracking(true);
    vp->setAutoFillBackground(false);

    // the apparently "magic" value of 20 is the same used internally in QScrollArea
    QScrollBar *sb = q->verticalScrollBar();
    sb->setSingleStep(20);
    q->connect(sb, SIGNAL(valueChanged(int)), q, SLOT(scrollValueChanged(int)));

    sb = q->horizontalScrollBar();
    sb->setSingleStep(20);
    q->connect(sb, SIGNAL(valueChanged(int)), q, SLOT(scrollValueChanged(int)));
}

GtDocViewPrivate::~GtDocViewPrivate()
{
}

void GtDocViewPrivate::setupCaches()
{
}

void GtDocViewPrivate::clearCaches()
{
}

void GtDocViewPrivate::changePage(int page)
{
    Q_Q(GtDocView);

    currentPage = page;
    pendingScroll = SCROLL_TO_PAGE_POSITION;

    QMetaObject::invokeMethod(q, "relayoutPages", Qt::QueuedConnection);
}

void GtDocViewPrivate::resizeContentArea(int width, int height)
{
    Q_Q(GtDocView);

    const QSize vs = q->viewport()->size();
    q->horizontalScrollBar()->setRange(0, width - vs.width());
    q->verticalScrollBar()->setRange(0, height - vs.height());
    updatePageStep();
}

void GtDocViewPrivate::updatePageStep()
{
    Q_Q(GtDocView);

    const QSize vs = q->viewport()->size();
    q->horizontalScrollBar()->setPageStep(vs.width());
    q->verticalScrollBar()->setPageStep(vs.height());
}

int GtDocViewPrivate::getPageYOffset(int page)
{
    return 0;
}

QSize GtDocViewPrivate::getMaxPageSize()
{
    double width = 0.;
    double height = 0.;

    if (document)
        document->getMaxPageSize(&width, &height);

    width *= scale;
    height *= scale;

    return (rotation % 180) ?
            QSize(height, width) :
            QSize(width, height);
}

QRect GtDocViewPrivate::computeBorder(const QSize &size)
{
    QRect rect(1, 1, 0, 0);

    if (size.width() < 100) {
        rect.setRight(2);
        rect.setBottom(2);
    }
    else if (size.width() < 500) {
        rect.setRight(3);
        rect.setBottom(3);
    }
    else {
        rect.setRight(4);
        rect.setBottom(4);
    }

    return rect;
}

QSize GtDocViewPrivate::layoutPagesContinuous()
{
    QSize size;
    size.setHeight(getPageYOffset(pageCount));

    switch (sizingMode) {
    case GtDocModel::FitWidth:
    case GtDocModel::BestFit:
        size.setWidth(0);
        break;

    case GtDocModel::FreeSize:
        {
            QSize maxSize = getMaxPageSize();
            QRect border = computeBorder(maxSize);

            size.setWidth(maxSize.width() + (spacing * 2) +
                          border.left() + border.right());
        }
        break;

    default:
        Q_ASSERT(0);
    }

    return size;
}

QSize GtDocViewPrivate::layoutPagesContinuousDualPage()
{
    Q_ASSERT(0);
    return QSize();
}

QSize GtDocViewPrivate::layoutPages()
{
    Q_ASSERT(0);
    return QSize();
}

QSize GtDocViewPrivate::layoutPagesDualPage()
{
    Q_ASSERT(0);
    return QSize();
}

GtDocView::GtDocView(QWidget *parent)
    : QAbstractScrollArea(parent)
    , d_ptr(new GtDocViewPrivate(this))
{
}

GtDocView::GtDocView(GtDocModel *model, QWidget *parent)
    : QAbstractScrollArea(parent)
    , d_ptr(new GtDocViewPrivate(this))
{
    setModel(model);
}

GtDocView::~GtDocView()
{
}

GtDocModel* GtDocView::model() const
{
    Q_D(const GtDocView);
    return d->model;
}

void GtDocView::setModel(GtDocModel *model)
{
    Q_D(GtDocView);

    if (model == d->model)
        return;

    if (d->model) {
        disconnect(d->model,
                   SIGNAL(documentChanged(GtDocument*)),
                   this,
                   SLOT(documentChanged(GtDocument*)));

        disconnect(d->model,
                   SIGNAL(pageChanged(int)),
                   this,
                   SLOT(pageChanged(int)));

        disconnect(d->model,
                   SIGNAL(scaleChanged(double)),
                   this,
                   SLOT(scaleChanged(double)));

        disconnect(d->model,
                   SIGNAL(rotationChanged(int)),
                   this,
                   SLOT(rotationChanged(int)));

        disconnect(d->model,
                   SIGNAL(continuousChanged(bool)),
                   this,
                   SLOT(continuousChanged(bool)));

        disconnect(d->model,
                   SIGNAL(layoutModeChanged(int)),
                   this,
                   SLOT(layoutModeChanged(int)));

        disconnect(d->model,
                   SIGNAL(sizingModeChanged(int)),
                   this,
                   SLOT(sizingModeChanged(int)));

        disconnect(d->model,
                   SIGNAL(destroyed(QObject*)),
                   this,
                   SLOT(modelDestroyed(QObject*)));
    }

    GtDocument *newdoc = NULL;
    d->model = model;
    if (d->model) {
        newdoc = model->document();
        d->rotation = model->rotation();
        d->scale = model->scale();
        d->continuous = model->continuous();
        d->layoutMode = model->layoutMode();
        d->sizingMode = model->sizingMode();

        connect(d->model,
                SIGNAL(documentChanged(GtDocument*)),
                this,
                SLOT(documentChanged(GtDocument*)));

        connect(d->model,
                SIGNAL(pageChanged(int)),
                this,
                SLOT(pageChanged(int)));

        connect(d->model,
                SIGNAL(scaleChanged(double)),
                this,
                SLOT(scaleChanged(double)));

        connect(d->model,
                SIGNAL(rotationChanged(int)),
                this,
                SLOT(rotationChanged(int)));

        connect(d->model,
                SIGNAL(continuousChanged(bool)),
                this,
                SLOT(continuousChanged(bool)));

        connect(d->model,
                SIGNAL(layoutModeChanged(int)),
                this,
                SLOT(layoutModeChanged(int)));

        connect(d->model,
                SIGNAL(sizingModeChanged(int)),
                this,
                SLOT(sizingModeChanged(int)));

        connect(d->model,
                SIGNAL(destroyed(QObject*)),
                this,
                SLOT(modelDestroyed(QObject*)));
    }

    documentChanged(newdoc);
}

bool GtDocView::canZoomIn() const
{
    return false;
}

void GtDocView::zoomIn()
{
}

bool GtDocView::canZoomOut() const
{
    return false;
}

void GtDocView::zoomOut()
{
}

void GtDocView::modelDestroyed(QObject *object)
{
    Q_D(GtDocView);

    if (object == static_cast<QObject *>(d->model))
        setModel(0);
}

void GtDocView::scrollValueChanged(int value)
{
    Q_UNUSED(value);
}

void GtDocView::documentChanged(GtDocument *document)
{
    Q_D(GtDocView);

    if (d->document == document)
        return;

    d->clearCaches();
    d->document = document;
    d->pageCount = document ? document->countPages() : 0;

    if (d->document) {
        d->setupCaches();

        int currentPage = d->model->page();
        if (d->currentPage != currentPage) {
            d->changePage(currentPage);
        }
        else {
            d->pendingScroll = GtDocViewPrivate::SCROLL_TO_KEEP_POSITION;
            QMetaObject::invokeMethod(this, "relayoutPages", Qt::QueuedConnection);
        }
    }
}

void GtDocView::pageChanged(int page)
{
}

void GtDocView::scaleChanged(double scale)
{
}

void GtDocView::rotationChanged(int rotation)
{
}

void GtDocView::continuousChanged(bool continuous)
{
}

void GtDocView::layoutModeChanged(int mode)
{
}

void GtDocView::sizingModeChanged(int mode)
{
}

void GtDocView::relayoutPages()
{
    Q_D(GtDocView);

    if (NULL == d->document) {
        d->resizeContentArea(0, 0);
        return;
    }

    if (d->continuous) {
        if (d->layoutMode == GtDocModel::SinglePage)
            d->layoutPagesContinuous();
        else
            d->layoutPagesContinuousDualPage();
    }
    else {
        if (d->layoutMode == GtDocModel::SinglePage)
            d->layoutPages();
        else
            d->layoutPagesDualPage();
    }
}

QPoint GtDocView::contentAreaPosition() const
{
    return QPoint(horizontalScrollBar()->value(),
                  verticalScrollBar()->value());
}

void GtDocView::resizeEvent(QResizeEvent *)
{
    qDebug() << "resize";
}

void GtDocView::keyPressEvent(QKeyEvent *)
{
    qDebug() << "key press";
}

void GtDocView::keyReleaseEvent(QKeyEvent *)
{
    qDebug() << "key release";
}

void GtDocView::inputMethodEvent(QInputMethodEvent *)
{
    qDebug() << "input method";
}

void GtDocView::wheelEvent(QWheelEvent *e)
{
    Q_D(GtDocView);

    if (NULL == d->document) {
        QAbstractScrollArea::wheelEvent(e);
        return;
    }

    int delta = e->delta();
    int vScroll = verticalScrollBar()->value();

    e->accept();
    Qt::KeyboardModifiers modifiers = e->modifiers();
    if ((modifiers & Qt::ControlModifier) == Qt::ControlModifier) {
        if (e->delta() < 0)
            zoomOut();
        else
            zoomIn();
    }
    else if ((modifiers & Qt::ShiftModifier) == Qt::ShiftModifier) {
    }
    else
        QAbstractScrollArea::wheelEvent(e);
}

void GtDocView::paintEvent(QPaintEvent *e)
{
    const QPoint areaPos = contentAreaPosition();

    // create the rect into contents from the clipped screen rect
    QRect viewportRect = viewport()->rect();
    viewportRect.translate(areaPos);

    QRect contentsRect = e->rect().translated(areaPos).intersected(viewportRect);
    if (!contentsRect.isValid())
        return;
}

GT_END_NAMESPACE
