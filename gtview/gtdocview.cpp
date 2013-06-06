/*
 * Copyright (C) 2013 Tom Wong. All rights reserved.
 */
#include "gtdocview.h"
#include "gtdocmodel.h"
#include "gtdocpage.h"
#include "gtdocument.h"
#include <QtCore/QDebug>
#include <QtCore/QTimer>
#include <QtGui/QPaintEvent>
#include <QtWidgets/QScrollBar>

GT_BEGIN_NAMESPACE

class GtDocViewPrivate
{
    Q_DECLARE_PUBLIC(GtDocView)

public:
    class HeightCache {
    public:
        HeightCache();
        ~HeightCache();

    public:
        void height(GtDocument *d, int page,
                    int r, int e, double s,
                    double *h, double *dh);
    private:
        void rebuild();

    private:
        GtDocument *document;
        int rotation;
        int evenPageLeft;
        double *heightToPage;
        double *dualHeightToPage;
    };

public:
    GtDocViewPrivate(GtDocView *parent);
    ~GtDocViewPrivate();

public:
    void changePage(int page);
    void resizeContentArea(const QSize &size);
    void updatePageStep();
    int evenPageLeft();
    void heightToPage(int page, double *height, double *dualHeight);
    int pageYOffset(int page);
    QSize maxPageSize();
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
    HeightCache heightCache;

    QTimer *delayResizeEventTimer;

    // prevent page repaint
    int lockPageRepaint;
    bool lockPageNeedRepaint;

    // infinite resizing loop prevention
    bool verticalScrollBarVisible;
};

GtDocViewPrivate::HeightCache::HeightCache()
    : document(0)
    , rotation(0)
    , evenPageLeft(0)
    , heightToPage(0)
    , dualHeightToPage(0)
{
}

GtDocViewPrivate::HeightCache::~HeightCache()
{
    delete[] heightToPage;
    delete[] dualHeightToPage;
}

void GtDocViewPrivate::HeightCache::height(GtDocument *d, int page,
                                           int r, int e, double s,
                                           double *h, double *dh)
{
    if (d != document || r != rotation || e != evenPageLeft) {
        document = d;
        rotation = r;
        evenPageLeft = e;

        rebuild();
    }

    if (h)
        *h = heightToPage[page] * s;

    if (dh)
        *dh = dualHeightToPage[page] * s;
}

void GtDocViewPrivate::HeightCache::rebuild()
{
    delete[] heightToPage;
    delete[] dualHeightToPage;

    if (!document) {
        heightToPage = 0;
        dualHeightToPage = 0;
        return;
    }

    GtDocPage *page;
    bool swap, uniform;
    int i, pageCount;
    double uniformHeight, pageHeight, nextPageHeight;
    double savedHeight;
    double uWidth, uHeight;

    swap = (rotation == 90 || rotation == 270);

    uniform = document->uniformPageSize(&uWidth, &uHeight);
    pageCount = document->pageCount();

    heightToPage = new double[pageCount + 1];
    dualHeightToPage = new double[pageCount + 2];
    savedHeight = 0;

    for (i = 0; i <= pageCount; ++i) {
        if (uniform) {
            uniformHeight = swap ? uWidth : uHeight;
            heightToPage[i] = i * uniformHeight;
        }
        else {
            if (i < pageCount) {
                double w, h;

                page = document->page(i);
                page->pageSize(&w, &h);
                pageHeight = swap ? w : h;
            }
            else {
                pageHeight = 0;
            }

            heightToPage[i] = savedHeight;
            savedHeight += pageHeight;
        }
    }

    if (evenPageLeft && !uniform) {
        double w, h;

        page = document->page(0);
        page->pageSize(&w, &h);
        savedHeight = swap ? w : h;
    }
    else {
        savedHeight = 0;
    }

    for (i = evenPageLeft; i < pageCount + 2; i += 2) {
        if (uniform) {
            uniformHeight = swap ? uWidth : uHeight;
            dualHeightToPage[i] = ((i + evenPageLeft) / 2) * uniformHeight;

            if (i + 1 < pageCount + 2)
                dualHeightToPage[i + 1] = ((i + evenPageLeft) / 2) * uniformHeight;
        }
        else {
            if (i + 1 < pageCount) {
                double w, h;

                page = document->page(i + 1);
                page->pageSize(&w, &h);
                nextPageHeight = swap ? w : h;
            }
            else {
                nextPageHeight = 0;
            }

            if (i < pageCount) {
                double w, h;

                page = document->page(i);
                page->pageSize(&w, &h);
                pageHeight = swap ? w : h;
            }
            else {
                pageHeight = 0;
            }

            if (i + 1 < pageCount + 2) {
                dualHeightToPage[i] = savedHeight;
                dualHeightToPage[i + 1] = savedHeight;
                savedHeight += MAX(pageHeight, nextPageHeight);
            }
            else {
                dualHeightToPage[i] = savedHeight;
            }
        }
    }
}

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
    , lockPageRepaint(0)
    , lockPageNeedRepaint(false)
    , verticalScrollBarVisible(false)

{
    Q_Q(GtDocView);

    delayResizeEventTimer = new QTimer(q);
    delayResizeEventTimer->setSingleShot(true);
    q->connect(delayResizeEventTimer, SIGNAL(timeout()), q, SLOT(delayedResizeEvent()));

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
    q->connect(sb, SIGNAL(valueChanged(int)), q, SLOT(repaintVisiblePages(int)));

    sb = q->horizontalScrollBar();
    sb->setSingleStep(20);
    q->connect(sb, SIGNAL(valueChanged(int)), q, SLOT(repaintVisiblePages(int)));

    q->setAttribute(Qt::WA_InputMethodEnabled, true);
}

GtDocViewPrivate::~GtDocViewPrivate()
{
}

void GtDocViewPrivate::changePage(int page)
{
    Q_Q(GtDocView);

    currentPage = page;
    pendingScroll = SCROLL_TO_PAGE_POSITION;

    QMetaObject::invokeMethod(q, "relayoutPages", Qt::QueuedConnection);
}

void GtDocViewPrivate::resizeContentArea(const QSize &size)
{
    Q_Q(GtDocView);

    const QSize vs = q->viewport()->size();
    q->horizontalScrollBar()->setRange(0, size.width() - vs.width());
    q->verticalScrollBar()->setRange(0, size.height() - vs.height());
    updatePageStep();
}

void GtDocViewPrivate::updatePageStep()
{
    Q_Q(GtDocView);

    const QSize vs = q->viewport()->size();
    q->horizontalScrollBar()->setPageStep(vs.width());
    q->verticalScrollBar()->setPageStep(vs.height());
}

int GtDocViewPrivate::evenPageLeft()
{
    return (layoutMode == GtDocModel::EvenPageLeft) ? 1 : 0;
}

void GtDocViewPrivate::heightToPage(int page, double *height, double *dualHeight)
{
    heightCache.height(document, page,
                       rotation, evenPageLeft(), scale,
                       height, dualHeight);
}

int GtDocViewPrivate::pageYOffset(int page)
{
    QSize maxSize = maxPageSize();
    QRect border = computeBorder(maxSize);
    double offset = 0;

    if (layoutMode != GtDocModel::SinglePage) {
        int e = evenPageLeft();
        heightToPage(page, NULL, &offset);
        offset += ((page + e) / 2 + 1) * spacing +
                  ((page + e) / 2 ) * (border.top() + border.bottom());
    }
    else {
        heightToPage(page, &offset, NULL);
        offset += (page + 1) * spacing + page * (border.top() + border.bottom());
    }

    return offset;
}

QSize GtDocViewPrivate::maxPageSize()
{
    double width = 0.;
    double height = 0.;

    if (document)
        document->maxPageSize(&width, &height);

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

    size.setHeight(pageYOffset(pageCount));

    switch (sizingMode) {
    case GtDocModel::FitWidth:
    case GtDocModel::BestFit:
        size.setWidth(0);
        break;

    case GtDocModel::FreeSize:
        {
            QSize maxSize = maxPageSize();
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

void GtDocView::lockPageRepaint()
{
    Q_D(GtDocView);
    d->lockPageRepaint++;
}

void GtDocView::unlockPageRepaint(bool repaint)
{
    Q_D(GtDocView);

    Q_ASSERT(d->lockPageRepaint > 0);

    if (repaint)
        d->lockPageNeedRepaint = true;

    d->lockPageRepaint--;
    if (0 == d->lockPageRepaint && d->lockPageNeedRepaint) {
        QMetaObject::invokeMethod(this, "repaintVisiblePages", Qt::QueuedConnection);
        d->lockPageNeedRepaint = false;
    }
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

void GtDocView::delayedResizeEvent()
{
    Q_D(GtDocView);

    d->delayResizeEventTimer->stop();
    relayoutPages();
    repaintVisiblePages();
}

void GtDocView::documentChanged(GtDocument *document)
{
    Q_D(GtDocView);

    if (d->document == document)
        return;

    d->document = document;
    d->pageCount = document ? document->pageCount() : 0;

    if (d->document) {
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
    Q_UNUSED(page);
}

void GtDocView::scaleChanged(double scale)
{
    Q_UNUSED(scale);
}

void GtDocView::rotationChanged(int rotation)
{
    Q_UNUSED(rotation);
}

void GtDocView::continuousChanged(bool continuous)
{
    Q_UNUSED(continuous);
}

void GtDocView::layoutModeChanged(int mode)
{
    Q_UNUSED(mode);
}

void GtDocView::sizingModeChanged(int mode)
{
    Q_UNUSED(mode);
}

void GtDocView::relayoutPages()
{
    Q_D(GtDocView);

    QSize size;
    if (NULL == d->document) {
        d->resizeContentArea(size);
        return;
    }

    if (d->continuous) {
        if (d->layoutMode == GtDocModel::SinglePage)
            size = d->layoutPagesContinuous();
        else
            size = d->layoutPagesContinuousDualPage();
    }
    else {
        if (d->layoutMode == GtDocModel::SinglePage)
            size = d->layoutPages();
        else
            size = d->layoutPagesDualPage();
    }

    d->resizeContentArea(size);
}

void GtDocView::repaintVisiblePages(int newValue)
{
    Q_D(GtDocView);

    if (d->lockPageRepaint > 0)
        return;
}

QPoint GtDocView::contentAreaPosition() const
{
    return QPoint(horizontalScrollBar()->value(),
                  verticalScrollBar()->value());
}

void GtDocView::resizeEvent(QResizeEvent *e)
{
    Q_D(GtDocView);

    if (!d->document) {
        d->resizeContentArea(e->size());
        return;
    }

    if (d->sizingMode == GtDocModel::FitWidth &&
        d->verticalScrollBarVisible && !verticalScrollBar()->isVisible() &&
        qAbs(e->oldSize().height() - e->size().height()) < verticalScrollBar()->width())
    {
        // this saves us from infinite resizing loop because of scrollbars appearing and disappearing
        // see bug 160628 for more info
        // TODO looks are still a bit ugly because things are left uncentered
        // but better a bit ugly than unusable
        d->verticalScrollBarVisible = false;
        d->resizeContentArea(e->size());
        return;
    }

    // start a timer that will refresh the pixmap after 0.2s
    d->delayResizeEventTimer->start(200);
    d->verticalScrollBarVisible = verticalScrollBar()->isVisible();
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
    Qt::KeyboardModifiers modifiers = e->modifiers();

    e->accept();
    if ((modifiers & Qt::ControlModifier) == Qt::ControlModifier) {
        if (delta < 0)
            zoomOut();
        else
            zoomIn();
    }
    else if ((modifiers & Qt::ShiftModifier) == Qt::ShiftModifier) {
    }
    else if (delta <= -120 && !d->continuous &&
             vScroll == verticalScrollBar()->maximum())
    {
        // go to next page
        if (d->currentPage + 1 < d->pageCount) {
            qDebug() << "next page";
        }
    }
    else if (delta >= 120 && !d->continuous &&
             vScroll == verticalScrollBar()->minimum())
    {
        // go to prev page
        if (d->currentPage > 0) {
            qDebug() << "prev page";
        }
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
