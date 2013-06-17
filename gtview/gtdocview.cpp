/*
 * Copyright (C) 2013 Tom Wong. All rights reserved.
 */
#include "gtdocview.h"
#include "gtdocmodel.h"
#include "gtdocpage.h"
#include "gtdocrange.h"
#include "gtdocrendercache.h"
#include "gtdocument.h"
#include <QtCore/QDebug>
#include <QtCore/QTimer>
#include <QtGui/QPainter>
#include <QtGui/QPaintEvent>
#include <QtWidgets/QScrollBar>
#include <math.h>

GT_BEGIN_NAMESPACE

#define ZOOM_IN_FACTOR  1.2
#define ZOOM_OUT_FACTOR (1.0/ZOOM_IN_FACTOR)

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
    explicit GtDocViewPrivate(GtDocView *parent);
    ~GtDocViewPrivate();

public:
    void changePage(int page);
    void resizeContentArea(const QSize &size);

    inline int evenPageLeft() {
        return (layoutMode == GtDocModel::EvenPageLeft) ? 1 : 0;
    }

    inline bool dualPage() {
        return (layoutMode != GtDocModel::SinglePage);
    }

    void heightToPage(int page, double *height, double *dualHeight);
    int pageYOffset(int page);
    QRect computeBorder(const QSize &size);

    inline QSize pageSizeOfView(int index) {
        return document->page(index)->size(scale, rotation);
    }

    inline QSize pageSizeOfDoc(int index) {
        return document->page(index)->size(1.0, rotation);
    }

    inline double zoomForSizeFitWidth(const QSize &docSize,
                                      const QSize &viewSize)
    {
        return (double)viewSize.width() / docSize.width();
    }

    inline double zoomForSizeBestFit(const QSize &docSize,
                                     const QSize &viewSize)
    {
        double wScale = (double)viewSize.width() / docSize.width();
        double hScale = (double)viewSize.height() / docSize.height();
        return MIN(wScale, hScale);
    }

    void pageExtents(int page, QRect *rect, QRect *border);
    QSize layoutPagesContinuous();
    QSize layoutPagesContinuousDualPage();
    QSize layoutPages();
    QSize layoutPagesDualPage();
    void zoomForSizeContinuousAndDualPage(int width, int height);
    void zoomForSizeContinuous(int width, int height);
    void zoomForSizeDualPage(int width, int height);
    void zoomForSizeSinglePage(int width, int height);

    void fillRegion(QPainter &p, const QRegion &r, const QColor &c);
    void drawPage(QPainter &p, int index, const QRect &pageArea,
                  const QRect &border, const QColor &backColor);
    QRegion textRegion(GtDocPage *page, int begin, int end);
    int pageDistance(int page, const QPoint &point, QPointF *ppoint);
    QTransform pageToViewMatrix(GtDocPage *page);
    GtDocPoint docPointFromViewPoint(const QPoint &p, bool inside);
    void updateCursor(const QPoint &p);

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
    GtDocModel::MouseMode mouseMode;
    HeightCache heightCache;

    QSharedPointer<GtDocRenderCache> renderCache;

    // selection
    GtDocPoint selectBegin;
    GtDocPoint selectEnd;

    // prevent update visible pages
    int lockPageUpdate;
    bool lockPageNeedUpdate;

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

    uniform = document->isPageSizeUniform();
    pageCount = document->pageCount();

    if (uniform) {
        if (pageCount > 0)
            document->page(0)->size(&uWidth, &uHeight);
        else
            qWarning() << "can't get uniform page size";
    }

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
                page->size(&w, &h);
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
        page->size(&w, &h);
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
                page->size(&w, &h);
                nextPageHeight = swap ? w : h;
            }
            else {
                nextPageHeight = 0;
            }

            if (i < pageCount) {
                double w, h;

                page = document->page(i);
                page->size(&w, &h);
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
    , mouseMode(GtDocModel::BrowseMode)
    , renderCache(new GtDocRenderCache(), &QObject::deleteLater)
    , lockPageUpdate(0)
    , lockPageNeedUpdate(false)
    , verticalScrollBarVisible(false)

{
    Q_Q(GtDocView);

    q->connect(renderCache.data(), SIGNAL(finished(int)), q, SLOT(renderFinished(int)));

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
    q->connect(sb, SIGNAL(valueChanged(int)), q, SLOT(updateVisiblePages(int)));

    sb = q->horizontalScrollBar();
    sb->setSingleStep(20);
    q->connect(sb, SIGNAL(valueChanged(int)), q, SLOT(updateVisiblePages(int)));

    q->setAttribute(Qt::WA_InputMethodEnabled, true);
}

GtDocViewPrivate::~GtDocViewPrivate()
{
}

void GtDocViewPrivate::changePage(int page)
{
    Q_Q(GtDocView);

    currentPage = page;

    QMetaObject::invokeMethod(q, "relayoutPages", Qt::QueuedConnection);
}

void GtDocViewPrivate::resizeContentArea(const QSize &size)
{
    Q_Q(GtDocView);

    const QSize vs = q->viewport()->size();

    q->horizontalScrollBar()->setRange(0, size.width() - vs.width());
    q->verticalScrollBar()->setRange(0, size.height() - vs.height());

    q->horizontalScrollBar()->setPageStep(vs.width());
    q->verticalScrollBar()->setPageStep(vs.height());
}

void GtDocViewPrivate::heightToPage(int page, double *height, double *dualHeight)
{
    heightCache.height(document, page,
                       rotation, evenPageLeft(), scale,
                       height, dualHeight);
}

int GtDocViewPrivate::pageYOffset(int page)
{
    QSize maxSize = document->maxPageSize(scale, rotation);
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

void GtDocViewPrivate::pageExtents(int page, QRect *rect, QRect *border)
{
    Q_Q(GtDocView);

    const QSize vs = q->viewport()->size();
    QSize pageSize;
    int x, y;

    /* Get the size of the page */
    pageSize = pageSizeOfView(page);
    *border = computeBorder(pageSize);
    rect->setWidth(pageSize.width() + border->left() + border->right());
    rect->setHeight(pageSize.height() + border->top() + border->bottom());

    if (continuous) {
        QSize maxSize = document->maxPageSize(scale, rotation);
        int maxWidth;

        maxWidth = maxSize.width() + border->left() + border->right();

        /* Get the location of the bounding box */
        if (dualPage()) {
            x = spacing + ((page % 2 == evenPageLeft()) ? 0 : 1) * (maxWidth + spacing);
            x = x + MAX(0, vs.width() - (maxWidth * 2 + spacing * 3)) / 2;
            if (page % 2 == evenPageLeft())
                x = x + (maxWidth - pageSize.width() - border->left() - border->right());
        }
        else {
            x = spacing;
            x = x + MAX(0, vs.width() - (pageSize.width() + spacing * 2)) / 2;
        }

        y = pageYOffset(page);
    }
    else {
        if (dualPage()) {
            int otherPage;
            QSize maxSize(pageSize);
            QSize otherSize;
            QRect overallBorder;

            otherPage = (page % 2 == evenPageLeft()) ? page + 1: page - 1;

            /* First, we get the bounding box of the two pages */
            if (otherPage < pageCount && (0 <= otherPage)) {
                otherSize = pageSizeOfView (otherPage);

                if (otherSize.width() > pageSize.width())
                    maxSize.setWidth(otherSize.width());

                if (otherSize.height() > pageSize.height())
                    maxSize.setHeight(otherSize.height());
            }

            overallBorder = computeBorder(maxSize);

            /* Find the offsets */
            x = spacing;
            y = spacing;

            /* Adjust for being the left or right page */
            if (page % 2 == evenPageLeft()) {
                x = x + maxSize.width() - pageSize.width();
            }
            else {
                x = x + (maxSize.width() +
                         overallBorder.left() +
                         overallBorder.right()) + spacing;
            }

            y = y + (maxSize.height() - pageSize.height()) / 2;

            /* Adjust for extra allocation */
            x = x + MAX(0, vs.width() -
                        ((maxSize.width() + overallBorder.left() + overallBorder.right()) * 2 + spacing * 3)) / 2;
            y = y + MAX(0, vs.height() - (pageSize.height() + spacing * 2)) / 2;
        }
        else {
            x = spacing;
            y = spacing;

            /* Adjust for extra allocation */
            x = x + MAX(0, vs.width() - (pageSize.width() + border->left() + border->right() + spacing * 2)) / 2;
            y = y + MAX(0, vs.height() - (pageSize.height() + border->top() + border->bottom() + spacing * 2)) / 2;
        }
    }

    rect->moveTo(x, y);
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
            QSize maxSize = document->maxPageSize(scale, rotation);
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

void GtDocViewPrivate::zoomForSizeContinuousAndDualPage(int width, int height)
{
    Q_Q(GtDocView);

    QSize docSize(document->maxPageSize(1.0, this->rotation));
    QRect border(computeBorder(docSize));

    docSize.setWidth(docSize.width() * 2);
    width -= (2 * (border.left() + border.right()) + 3 * spacing);
    height -= (border.top() + border.bottom() + 2 * spacing - 1);

    QSize viewSize(width - q->verticalScrollBar()->width(), height);

    if (sizingMode == GtDocModel::FitWidth) {
        model->setScale(zoomForSizeFitWidth(docSize, viewSize));
    }
    else if (sizingMode == GtDocModel::BestFit) {
        model->setScale(zoomForSizeBestFit(docSize, viewSize));
    }
    else {
        Q_ASSERT(0);
    }
}

void GtDocViewPrivate::zoomForSizeContinuous(int width, int height)
{
    Q_Q(GtDocView);

    QSize docSize(document->maxPageSize(1.0, this->rotation));
    QRect border(computeBorder(docSize));

    width -= (border.left() + border.right() + 2 * spacing);
    height -= (border.top() + border.bottom() + 2 * spacing - 1);

    QSize viewSize(width - q->verticalScrollBar()->width(), height);

    if (sizingMode == GtDocModel::FitWidth) {
        model->setScale(zoomForSizeFitWidth(docSize, viewSize));
    }
    else if (sizingMode == GtDocModel::BestFit) {
        model->setScale(zoomForSizeBestFit(docSize, viewSize));
    }
    else {
        Q_ASSERT(0);
    }
}

void GtDocViewPrivate::zoomForSizeDualPage(int width, int height)
{
    Q_Q(GtDocView);

    /* Find the largest of the two. */
    QSize docSize(pageSizeOfDoc(currentPage));
    int otherPage = currentPage ^ 1;

    if (otherPage < pageCount) {
        QSize docSize2(pageSizeOfDoc(otherPage));

        if (docSize2.width() > docSize.width())
            docSize.setWidth(docSize2.width());

        if (docSize2.height() > docSize.height())
            docSize.setHeight(docSize2.height());
    }

    QRect border(computeBorder(QSize(width, height)));

    docSize.setWidth(docSize.width() * 2);
    width -= ((border.left() + border.right())* 2 + 3 * spacing);
    height -= (border.top() + border.bottom() + 2 * spacing);

    if (sizingMode == GtDocModel::FitWidth) {
        QSize viewSize(width - q->verticalScrollBar()->width(), height);
        model->setScale(zoomForSizeFitWidth(docSize, viewSize));
    }
    else if (sizingMode == GtDocModel::BestFit) {
        QSize viewSize(width, height);
        model->setScale(zoomForSizeBestFit(docSize, viewSize));
    }
    else {
        Q_ASSERT(0);
    }
}

void GtDocViewPrivate::zoomForSizeSinglePage(int width, int height)
{
    Q_Q(GtDocView);

    QSize docSize(pageSizeOfDoc(currentPage));
    QRect border(computeBorder(QSize(width, height)));

    width -= (border.left() + border.right() + 2 * spacing);
    height -= (border.top() + border.bottom() + 2 * spacing);

    if (sizingMode == GtDocModel::FitWidth) {
        QSize viewSize(width - q->verticalScrollBar()->width(), height);
        model->setScale(zoomForSizeFitWidth(docSize, viewSize));
    }
    else if (sizingMode == GtDocModel::BestFit) {
        QSize viewSize(width, height);
        model->setScale(zoomForSizeBestFit(docSize, viewSize));
    }
    else {
        Q_ASSERT(0);
    }
}

void GtDocViewPrivate::fillRegion(QPainter &p, const QRegion &r, const QColor &c)
{
    const QVector<QRect> &rects = r.rects();
    int n = rects.count();

    for (int i = 0; i < n; ++i)
        p.fillRect(rects[i], c);
}

void GtDocViewPrivate::drawPage(QPainter &p, int index, const QRect &pageArea,
                                const QRect &border, const QColor &backColor)
{
    QRect realArea(pageArea.x() + border.left(),
                   pageArea.y() + border.top(),
                   pageArea.width() - border.left() - border.right(),
                   pageArea.height() - border.top() - border.bottom());

    // draw border and background
    int levels = border.right() - border.left();
    int x = realArea.x();
    int y = realArea.y();
    int width = realArea.width();
    int height = realArea.height();

    p.setPen(Qt::black);
    p.drawRect(x - 1, y - 1, width + 1, height + 1);

    // draw bottom/right gradient
    int r = backColor.red() / (levels + 2) + 6;
    int g = backColor.green() / (levels + 2) + 6;
    int b = backColor.blue() / (levels + 2) + 6;

    p.translate(x, y);
    for (int i = 0; i < levels; i++) {
        p.setPen(QColor(r * (i+2), g * (i+2), b * (i+2)));
        p.drawLine(i, i + height + 1, i + width + 1, i + height + 1);
        p.drawLine(i + width + 1, i, i + width + 1, i + height);
        p.setPen(backColor);
        p.drawLine(-1, i + height + 1, i - 1, i + height + 1);
        p.drawLine(i + width + 1, -1, i + width + 1, i - 1);
    }

    p.translate(-x, -y);

    // draw page contents
    QImage image = renderCache->image(index);
    if (!image.isNull())
        p.drawImage(realArea, image);
    else
        p.fillRect(realArea, QColor(255, 255, 255));
}

QRegion GtDocViewPrivate::textRegion(GtDocPage *page, int begin, int end)
{
    const QSharedDataPointer<GtDocText> text(page->text());
    const QRectF *rect = text->rects() + begin;
    QTransform m = pageToViewMatrix(page);
    QRegion region;
    QRectF lineRect;
    QRectF temp;

    for (int i = begin; i < end; ++i, ++rect) {
        if (lineRect.isValid()) {
            if (qAbs(lineRect.top() - rect->top()) < 4.0 &&
                qAbs(lineRect.bottom() - rect->bottom()) < 4.0)
            {
                if (rect->left() < lineRect.left())
                    lineRect.setLeft(rect->left());

                if (rect->right() > lineRect.right())
                    lineRect.setRight(rect->right());
            }
            else {
                temp = m.mapRect(lineRect);
                region += QRect(temp.x(), temp.y(), temp.width(), temp.height());
                lineRect = *rect;
            }
        }
        else {
            lineRect = *rect;
        }
    }

    temp = m.mapRect(lineRect);
    region += QRect(temp.x(), temp.y(), temp.width(), temp.height());
    return region;
}

int GtDocViewPrivate::pageDistance(int page, const QPoint &point, QPointF *ppoint)
{
    QRect rect, border;
    pageExtents(page, &rect, &border);

    int x = point.x();
    int y = point.y();
    int l = rect.left();
    int t = rect.top();
    int r = rect.right();
    int b = rect.bottom();
    int d = 0;

    if (x < l) {
        ppoint->setX(0);
        if (y < t) {
            d = hypot(l - x, t - y);
            ppoint->setY(0);
        }
        else if (y >= b) {
            d = hypot(l - x, y - b);
            ppoint->setY(b - t);
        }
        else {
            d = l - x;
            ppoint->setY(y - t);
        }
    }
    else if (x >= r) {
        ppoint->setX(r - l);
        if (y < t) {
            d = hypot(x - r, t - y);
            ppoint->setY(0);
        }
        else if (y >= b) {
            d = hypot(x - r, y - b);
            ppoint->setY(b - t);
        }
        else {
            d = x - r;
            ppoint->setY(y - t);
        }
    }
    else if (y < t) {
        d = t - y;
        ppoint->setX(x - l);
        ppoint->setY(0);
    }
    else if (y >= b) {
        d = y - b;
        ppoint->setX(x - l);
        ppoint->setY(b - t);
    }
    else {
        ppoint->setX(x - l);
        ppoint->setY(y - t);
    }

    return d;
}

QTransform GtDocViewPrivate::pageToViewMatrix(GtDocPage *page)
{
    QTransform m;
    qreal w, h;

    page->size(&w, &h);
    w *= scale;
    h *= scale;

    if (90 == rotation) {
        m = m.translate(h, 0);
    }
    else if (180 == rotation) {
        m = m.translate(w, h);
    }
    else if (270 == rotation) {
        m = m.translate(0, w);
    }
    else {
        Q_ASSERT(rotation == 0);
    }

    m = m.scale(scale, scale);
    return m.rotate(rotation);
}

GtDocPoint GtDocViewPrivate::docPointFromViewPoint(const QPoint &p, bool inside)
{
    Q_Q(GtDocView);

    QPoint point(p + q->scrollPoint());
    QPointF ppoint;
    int bestPage = -1;
    int bestDistance = -1;
    int distance;

    for (int i = beginPage; i < endPage; ++i) {
        distance = pageDistance(i, point, &ppoint);
        if (-1 == bestPage || distance < bestDistance) {
            bestPage = i;
            bestDistance = distance;

            // point inside page
            if (0 == bestDistance)
                break;
        }
    }

    if (inside && bestDistance != 0)
        return GtDocPoint();

    GtDocPage *page = document->page(bestPage);
    QTransform m = pageToViewMatrix(page);
    m = m.inverted();
    return GtDocPoint(page, m.map(ppoint));
}

void GtDocViewPrivate::updateCursor(const QPoint &p)
{
    Q_Q(GtDocView);

    if (GtDocModel::SelectText == mouseMode) {
        GtDocPoint docPoint = docPointFromViewPoint(p, true);

        if (docPoint.offset(true) != -1)
            q->setCursor(Qt::IBeamCursor);
        else
            q->setCursor(Qt::ArrowCursor);
    }
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
                   SIGNAL(mouseModeChanged(int)),
                   this,
                   SLOT(mouseModeChanged(int)));

        disconnect(d->model,
                   SIGNAL(destroyed(QObject*)),
                   this,
                   SLOT(modelDestroyed(QObject*)));
    }

    GtDocument *newdoc = NULL;
    d->model = model;
    d->renderCache->setModel(d->model);

    if (d->model) {
        newdoc = model->document();
        d->rotation = model->rotation();
        d->scale = model->scale();
        d->continuous = model->continuous();
        d->layoutMode = model->layoutMode();
        d->sizingMode = model->sizingMode();
        d->mouseMode = model->mouseMode();

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
                SIGNAL(mouseModeChanged(int)),
                this,
                SLOT(mouseModeChanged(int)));

        connect(d->model,
                SIGNAL(destroyed(QObject*)),
                this,
                SLOT(modelDestroyed(QObject*)));
    }

    documentChanged(newdoc);
}

void GtDocView::setRenderThread(QThread *thread)
{
    Q_D(GtDocView);
    d->renderCache->moveToThread(thread);
}

void GtDocView::setRenderCacheSize(int size)
{
    Q_D(GtDocView);
    d->renderCache->setMaxSize(size);
}

void GtDocView::lockPageUpdate()
{
    Q_D(GtDocView);
    d->lockPageUpdate++;
}

void GtDocView::unlockPageUpdate(bool update)
{
    Q_D(GtDocView);

    Q_ASSERT(d->lockPageUpdate > 0);

    if (update)
        d->lockPageNeedUpdate = true;

    d->lockPageUpdate--;
    if (0 == d->lockPageUpdate && d->lockPageNeedUpdate) {
        QMetaObject::invokeMethod(this, "updateVisiblePages", Qt::QueuedConnection);
        d->lockPageNeedUpdate = false;
    }
}

bool GtDocView::canZoomIn() const
{
    Q_D(const GtDocView);

    if (d->model)
        return d->scale < d->model->maxScale();

    return false;
}

void GtDocView::zoomIn()
{
    Q_D(GtDocView);

    double scale = d->model->scale() * ZOOM_IN_FACTOR;
    d->model->setSizingMode(GtDocModel::FreeSize);
    d->model->setScale(scale);
}

bool GtDocView::canZoomOut() const
{
    Q_D(const GtDocView);

    if (d->model)
        return d->scale > d->model->minScale();

    return false;
}

void GtDocView::zoomOut()
{
    Q_D(GtDocView);

    double scale = d->model->scale() * ZOOM_OUT_FACTOR;
    d->model->setSizingMode(GtDocModel::FreeSize);
    d->model->setScale(scale);
}

QPoint GtDocView::scrollPoint() const
{
    return QPoint(horizontalScrollBar()->value(), verticalScrollBar()->value());
}

void GtDocView::scrollTo(int x, int y)
{
    lockPageUpdate();
    horizontalScrollBar()->setValue(x);
    verticalScrollBar()->setValue(y);
    unlockPageUpdate();
}

void GtDocView::renderFinished(int page)
{
    Q_D(GtDocView);

    QRect pageArea;
    QRect border;
    int scrollX = horizontalScrollBar()->value();
    int scrollY = verticalScrollBar()->value();

    d->pageExtents(page, &pageArea, &border);
    viewport()->repaint(pageArea.translated(-scrollX, -scrollY));
}

void GtDocView::modelDestroyed(QObject *object)
{
    Q_D(GtDocView);

    if (object == static_cast<QObject *>(d->model))
        setModel(0);
}

void GtDocView::documentChanged(GtDocument *document)
{
    Q_D(GtDocView);

    if (d->document == document)
        return;

    d->document = document;
    d->pageCount = document ? document->pageCount() : 0;
    d->renderCache->clear();
    d->selectBegin = GtDocPoint();
    d->selectEnd = GtDocPoint();

    if (d->document) {
        int currentPage = d->model->page();
        if (d->currentPage != currentPage) {
            d->changePage(currentPage);
        }
        else {
            QMetaObject::invokeMethod(this, "relayoutPages", Qt::QueuedConnection);
        }
    }
    else {
        QMetaObject::invokeMethod(this, "relayoutPages", Qt::QueuedConnection);
    }
}

void GtDocView::pageChanged(int page)
{
    Q_D(GtDocView);

    if (!d->document)
        return;

    if (d->currentPage != page) {
        d->changePage(page);
    }
    else {
        QMetaObject::invokeMethod(this, "relayoutPages", Qt::QueuedConnection);
    }
}

void GtDocView::scaleChanged(double scale)
{
    Q_D(GtDocView);

    if (ABS(d->scale - scale) < 0.0000001)
        return;

    d->scale = scale;

    if (d->sizingMode == GtDocModel::FreeSize)
        QMetaObject::invokeMethod(this, "relayoutPages", Qt::QueuedConnection);
}

void GtDocView::rotationChanged(int rotation)
{
    Q_D(GtDocView);

    d->rotation = rotation;
    d->renderCache->clear();

    QMetaObject::invokeMethod(this, "relayoutPages", Qt::QueuedConnection);
}

void GtDocView::continuousChanged(bool continuous)
{
    Q_D(GtDocView);

    d->continuous = continuous;

    QMetaObject::invokeMethod(this, "relayoutPages", Qt::QueuedConnection);
}

void GtDocView::layoutModeChanged(int mode)
{
    Q_D(GtDocView);

    d->layoutMode = static_cast<GtDocModel::LayoutMode>(mode);

    /* FIXME: if we're keeping the pixbuf cache around, we should
     * extend the preload_cache_size to be 2 if dual_page is set.
     */
    QMetaObject::invokeMethod(this, "relayoutPages", Qt::QueuedConnection);
}

void GtDocView::sizingModeChanged(int mode)
{
    Q_D(GtDocView);

    d->sizingMode = static_cast<GtDocModel::SizingMode>(mode);

    if (mode != GtDocModel::FreeSize)
        QMetaObject::invokeMethod(this, "relayoutPages", Qt::QueuedConnection);
}

void GtDocView::mouseModeChanged(int mode)
{
    Q_D(GtDocView);

    d->mouseMode = static_cast<GtDocModel::MouseMode>(mode);

    if (d->selectBegin.isValid() || d->selectEnd.isValid()) {
        d->selectBegin = GtDocPoint();
        d->selectEnd = GtDocPoint();
        viewport()->update();
    }
}

void GtDocView::relayoutPages()
{
    Q_D(GtDocView);

    QSize size;
    if (NULL == d->document) {
        d->resizeContentArea(size);
        return;
    }

    QScrollBar *hsbar = horizontalScrollBar();
    QScrollBar *vsbar = verticalScrollBar();
    double dx = (double)hsbar->value() / hsbar->maximum();
    double dy = (double)vsbar->value() / vsbar->maximum();

    if (d->sizingMode != GtDocModel::FreeSize) {
        int width = viewport()->width();
        int height = viewport()->height();

        if (d->continuous && d->dualPage())
            d->zoomForSizeContinuousAndDualPage(width, height);
        else if (d->continuous)
            d->zoomForSizeContinuous(width, height);
        else if (d->dualPage())
            d->zoomForSizeDualPage(width, height);
        else
            d->zoomForSizeSinglePage(width, height);
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

    scrollTo(hsbar->maximum() * dx, vsbar->maximum() * dy);
}

void GtDocView::updateVisiblePages(int newValue)
{
    Q_D(GtDocView);

    if (d->lockPageUpdate > 0)
        return;

    int begin = d->beginPage;
    int end = d->endPage;

    if (!d->document) {
        d->beginPage = -1;
        d->endPage = -1;
        d->currentPage = -1;
    }
    else if (d->continuous) {
        QRect unused, pageArea, border;
        bool found = false;
        int areaMax = -1, area;
        int bestCurrentPage = -1;
        int i, j = 0;

        const QRect viewportRect(horizontalScrollBar()->value(),
                                 verticalScrollBar()->value(),
                                 viewport()->width(),
                                 viewport()->height());

        for (i = 0; i < d->pageCount; ++i) {
            d->pageExtents(i, &pageArea, &border);

            unused = viewportRect.intersected(pageArea);
            if (!unused.isEmpty()) {
                area = unused.width() * unused.height();

                if (!found) {
                    areaMax = area;
                    d->beginPage = i;
                    found = true;
                    bestCurrentPage = i;
                }

                if (area > areaMax) {
                    bestCurrentPage = (area == areaMax) ? MIN(i, bestCurrentPage) : i;
                    areaMax = area;
                }

                d->endPage = i + 1;
                j = 0;
            }
            else if (found && d->currentPage < d->endPage) {
                if (d->dualPage() && j < 1) {
                    /* In dual mode we stop searching
                     * after two consecutive non-visible pages.
                     */
                    j++;
                    continue;
                }
                break;
            }
        }

        bestCurrentPage = MAX(bestCurrentPage, d->beginPage);

        if (d->currentPage != bestCurrentPage) {
            d->currentPage = bestCurrentPage;
            d->model->setPage(bestCurrentPage);
        }
    }
    else if (d->dualPage()) {
        if (d->currentPage % 2 == d->evenPageLeft()) {
            d->beginPage = d->currentPage;

            if (d->currentPage + 1 < d->pageCount)
                d->endPage = d->beginPage + 2;
            else
                d->endPage = d->beginPage + 1;
        }
        else {
            if (d->currentPage < 1)
                d->beginPage = d->currentPage;
            else
                d->beginPage = d->currentPage - 1;

            d->endPage = d->currentPage + 1;
        }
    }
    else {
        d->beginPage = d->currentPage;
        d->endPage = d->currentPage + 1;
    }

    if (d->beginPage == -1 || d->endPage == -1)
        return;

    if (begin != d->beginPage || end != d->endPage) {
    }

    d->renderCache->setPageRange(d->beginPage, d->endPage);

    if (-1 == newValue)
        viewport()->update();
}

void GtDocView::resizeEvent(QResizeEvent *e)
{
    Q_D(GtDocView);

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

    d->verticalScrollBarVisible = verticalScrollBar()->isVisible();
    QMetaObject::invokeMethod(this, "relayoutPages", Qt::QueuedConnection);
}

void GtDocView::keyPressEvent(QKeyEvent *)
{
}

void GtDocView::keyReleaseEvent(QKeyEvent *)
{
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
        if (delta < 0) {
            if (canZoomOut())
                zoomOut();
        }
        else {
            if (canZoomIn())
                zoomIn();
        }
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
    Q_D(GtDocView);

    QRect viewportRect = viewport()->rect();
    QRect contentsRect = e->rect().intersected(viewportRect);
    if (!contentsRect.isValid())
        return;

    QPainter p(viewport());
    int scrollX = horizontalScrollBar()->value();
    int scrollY = verticalScrollBar()->value();
    QRegion remainingArea(contentsRect);
    QColor backColor = viewport()->palette().color(QPalette::Dark);
    QColor selBgColor = QColor(30, 76, 100, 120);
    GtDocRange selectRange;

    if (d->selectEnd > d->selectBegin) {
        selectRange.setPoints(d->selectBegin, d->selectEnd);
    }
    else {
        selectRange.setPoints(d->selectEnd, d->selectBegin);
    }

    // draw page contents
    QRect pageArea, border, overlap;
    for (int i = d->beginPage; i < d->endPage; ++i) {
        d->pageExtents(i, &pageArea, &border);
        pageArea.translate(-scrollX, -scrollY);
        overlap = contentsRect.intersected(pageArea);

        if (!overlap.isValid())
            continue;

        d->drawPage(p, i, pageArea, border, backColor);
        remainingArea -= pageArea;

        // highlight selected text
        if (GtDocModel::SelectText == d->mouseMode) {
            GtDocPage *page = d->document->page(i);
            QPoint selText(selectRange.intersectedText(page));
            QRegion textRgn = d->textRegion(page, selText.x(), selText.y());

            textRgn.translate(pageArea.x() + border.left(),
                              pageArea.y() + border.top());

            d->fillRegion(p, textRgn, selBgColor);
        }
    }

    if (d->selectBegin.isValid() && d->selectEnd.isValid()) {
    }

    // fill with background color the unpainted area
    d->fillRegion(p, remainingArea, backColor);
}

void GtDocView::mouseMoveEvent(QMouseEvent *e)
{
    Q_D(GtDocView);

    if (e->buttons() == Qt::LeftButton) {
        if (d->selectBegin.isValid()) {
            GtDocPoint docPoint = d->docPointFromViewPoint(e->pos(), false);
            if (docPoint != d->selectEnd) {
                d->selectEnd = docPoint;
                // TODO: update invalid region only
                viewport()->update();
            }
        }
    }
    else {
        d->updateCursor(e->pos());
    }
}

void GtDocView::mousePressEvent(QMouseEvent *e)
{
    Q_D(GtDocView);

    if (d->document && d->mouseMode != GtDocModel::BrowseMode) {
        d->selectBegin = d->docPointFromViewPoint(e->pos(), true);
        d->selectEnd = GtDocPoint();
    }
}

void GtDocView::mouseReleaseEvent(QMouseEvent *e)
{
    Q_D(GtDocView);

    d->updateCursor(e->pos());
}

void GtDocView::mouseDoubleClickEvent(QMouseEvent *)
{
}

bool GtDocView::viewportEvent(QEvent *e)
{
    return QAbstractScrollArea::viewportEvent(e);
}

GT_END_NAMESPACE
