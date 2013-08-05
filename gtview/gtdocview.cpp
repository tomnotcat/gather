/*
 * Copyright (C) 2013 Tom Wong. All rights reserved.
 */
#include "gtdocview.h"
#include "gtbookmarks.h"
#include "gtdoccommand.h"
#include "gtdocmodel.h"
#include "gtdocnote.h"
#include "gtdocnotes.h"
#include "gtdocpage.h"
#include "gtdocrange.h"
#include "gtdocrendercache.h"
#include "gtdocument.h"
#include "gtlinkdest.h"
#include <QtCore/QDebug>
#include <QtCore/QTimer>
#include <QtGui/QClipboard>
#include <QtGui/QPainter>
#include <QtGui/QPaintEvent>
#include <QtWidgets/QApplication>
#include <QtWidgets/QScrollBar>
#include <QtWidgets/QUndoStack>
#include <math.h>

GT_BEGIN_NAMESPACE

#define ZOOM_IN_FACTOR  (1.2)
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
    explicit GtDocViewPrivate(GtDocView *q);
    ~GtDocViewPrivate();

public:
    void connectSyncSignals();
    void disconnectSyncSignals();
    void resizeContentArea(const QSize &size);

    inline bool isDocLoaded() const {
        return m_document && m_document->isLoaded();
    }

    inline bool hasSyncFlags(GtDocView::SyncFlags flags) const {
        return (m_syncFlags & flags) == flags;
    }

    inline int evenPageLeft() const {
        return (m_layoutMode == GtDocModel::EvenPageLeft) ? 1 : 0;
    }

    inline bool dualPage() const {
        return (m_layoutMode != GtDocModel::SinglePage);
    }

    void heightToPage(int page, double *height, double *dualHeight);
    int pageYOffset(int page);
    QRect computeBorder(const QSize &size);

    inline QSize pageSizeOfView(int index) {
        return m_document->page(index)->size(m_scale, m_rotation);
    }

    inline QSize pageSizeOfDoc(int index) {
        return m_document->page(index)->size(1.0, m_rotation);
    }

    inline double zoomForSizeFitWidth(const QSize &docSize,
                                      const QSize &viewSize) const
    {
        return (double)viewSize.width() / docSize.width();
    }

    inline double zoomForSizeBestFit(const QSize &docSize,
                                     const QSize &viewSize) const
    {
        double wScale = (double)viewSize.width() / docSize.width();
        double hScale = (double)viewSize.height() / docSize.height();
        return MIN(wScale, hScale);
    }

    QRect pageExtents(int page, QRect *border = 0);
    QSize layoutPagesContinuous();
    QSize layoutPagesContinuousDualPage();
    QSize layoutPages();
    QSize layoutPagesDualPage();
    void zoomForSizeContinuousAndDualPage(int width, int height);
    void zoomForSizeContinuous(int width, int height);
    void zoomForSizeDualPage(int width, int height);
    void zoomForSizeSinglePage(int width, int height);

    void fillRegion(QPainter &p, const QRegion &r, const QColor &c);
    void drawUnderline(QPainter &p, const QVector<QRect> &rs,
                       const QPoint &offset);
    void drawPageNote(QPainter &p, GtDocPage *page,
                      GtDocNote *note, const QPoint &offset);
    void drawPage(QPainter &p, int index, const QRect &pageArea,
                  const QRect &border, const GtDocRange &selRange);
    QVector<QRect> textRects(GtDocPage *page, int begin, int end) const;
    QRegion rangeRegion(const GtDocRange &range, GtDocPage *page) const;
    int pageDistance(int page, const QPoint &point, QPointF *ppoint);
    QTransform pageAreaToView(GtDocPage *page) const;
    QPoint pageViewToView(int index);
    GtDocPoint docPointFromViewPoint(const QPoint &p, bool inside);
    QPoint viewPointFromDocPoint(const GtDocPoint &p);
    void updateCursor(const QPoint &p);
    void relayoutPagesLater();
    void repaintDocRange(const GtDocRange &range);
    void repaintOldAndNewSelection(const GtDocRange &oldRange);

private:
    GtDocView *q_ptr;
    GtDocModel *m_model;
    QUndoStack *m_undoStack;
    GtDocument *m_document;
    GtBookmarks *m_bookmarks;
    GtDocNotes *m_notes;
    GtDocView::SyncFlags m_syncFlags;
    int m_beginPage;
    int m_endPage;
    int m_currentPage;
    int m_pageCount;
    int m_rotation;
    int m_spacing;
    double m_scale;
    bool m_continuous;
    GtDocModel::LayoutMode m_layoutMode;
    GtDocModel::SizingMode m_sizingMode;
    GtDocModel::MouseMode m_mouseMode;
    HeightCache m_heightCache;

    QSharedPointer<GtDocRenderCache> m_renderCache;
    QBasicTimer m_cursorBlinkTimer;

    // selection
    GtDocPoint m_selectBegin;
    GtDocPoint m_selectEnd;
    QColor m_paperColor;
    QColor m_backColor;
    QColor m_highlightColor;
    QColor m_underlineColor;
    QColor m_selBgColor;
    qreal m_underlineWidth;

    // prevent update visible pages
    int m_lockPageUpdate;
    bool m_lockPageNeedUpdate;

    // infinite resizing loop prevention
    bool m_verticalScrollBarVisible;
    bool m_pendingRelayoutPages;
    bool m_selectWordOnDoubleClick;
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

GtDocViewPrivate::GtDocViewPrivate(GtDocView *q)
    : q_ptr(q)
    , m_model(0)
    , m_undoStack(0)
    , m_document(0)
    , m_bookmarks(0)
    , m_notes(0)
    , m_syncFlags(GtDocView::SyncNone)
    , m_beginPage(-1)
    , m_endPage(-1)
    , m_currentPage(-1)
    , m_pageCount(0)
    , m_rotation(0)
    , m_spacing(5)
    , m_scale(1.0)
    , m_continuous(true)
    , m_layoutMode(GtDocModel::SinglePage)
    , m_sizingMode(GtDocModel::FitWidth)
    , m_mouseMode(GtDocModel::BrowseMode)
    , m_renderCache(new GtDocRenderCache(q), &QObject::deleteLater)
    , m_paperColor(255, 255, 255)
    , m_highlightColor(255, 255, 0)
    , m_underlineColor(255, 64, 64)
    , m_selBgColor(30, 76, 100, 120)
    , m_underlineWidth(1.5)
    , m_lockPageUpdate(0)
    , m_lockPageNeedUpdate(false)
    , m_verticalScrollBarVisible(false)
    , m_pendingRelayoutPages(false)
    , m_selectWordOnDoubleClick(false)

{
    m_backColor = q->viewport()->palette().color(QPalette::Dark);

    q->connect(m_renderCache.data(), SIGNAL(finished(int)),
               q, SLOT(renderFinished(int)));

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
    q->connect(sb, SIGNAL(valueChanged(int)),
               q, SLOT(updateVisiblePages(int)));

    sb = q->horizontalScrollBar();
    sb->setSingleStep(20);
    q->connect(sb, SIGNAL(valueChanged(int)),
               q, SLOT(updateVisiblePages(int)));

    q->setAttribute(Qt::WA_InputMethodEnabled, true);
}

GtDocViewPrivate::~GtDocViewPrivate()
{
}

void GtDocViewPrivate::connectSyncSignals()
{
    Q_Q(GtDocView);

    if (hasSyncFlags(GtDocView::SyncScale)) {
        q->connect(m_model,
                   SIGNAL(scaleChanged(double)),
                   q,
                   SLOT(scaleChanged(double)));
    }

    if (hasSyncFlags(GtDocView::SyncRotation)) {
        q->connect(m_model,
                   SIGNAL(rotationChanged(int)),
                   q,
                   SLOT(rotationChanged(int)));
    }

    if (hasSyncFlags(GtDocView::SyncContinuous)) {
        q->connect(m_model,
                   SIGNAL(continuousChanged(bool)),
                   q,
                   SLOT(continuousChanged(bool)));
    }

    if (hasSyncFlags(GtDocView::SyncLayoutMode)) {
        q->connect(m_model,
                   SIGNAL(layoutModeChanged(int)),
                   q,
                   SLOT(layoutModeChanged(int)));
    }

    if (hasSyncFlags(GtDocView::SyncSizingMode)) {
        q->connect(m_model,
                   SIGNAL(sizingModeChanged(int)),
                   q,
                   SLOT(sizingModeChanged(int)));
    }

    if (hasSyncFlags(GtDocView::SyncMouseMode)) {
        q->connect(m_model,
                   SIGNAL(mouseModeChanged(int)),
                   q,
                   SLOT(mouseModeChanged(int)));
    }
}

void GtDocViewPrivate::disconnectSyncSignals()
{
    Q_Q(GtDocView);

    if (hasSyncFlags(GtDocView::SyncScale)) {
        q->disconnect(m_model,
                      SIGNAL(scaleChanged(double)),
                      q,
                      SLOT(scaleChanged(double)));
    }

    if (hasSyncFlags(GtDocView::SyncRotation)) {
        q->disconnect(m_model,
                      SIGNAL(rotationChanged(int)),
                      q,
                      SLOT(rotationChanged(int)));
    }

    if (hasSyncFlags(GtDocView::SyncContinuous)) {
        q->disconnect(m_model,
                      SIGNAL(continuousChanged(bool)),
                      q,
                      SLOT(continuousChanged(bool)));
    }

    if (hasSyncFlags(GtDocView::SyncLayoutMode)) {
        q->disconnect(m_model,
                      SIGNAL(layoutModeChanged(int)),
                      q,
                      SLOT(layoutModeChanged(int)));
    }

    if (hasSyncFlags(GtDocView::SyncSizingMode)) {
        q->disconnect(m_model,
                      SIGNAL(sizingModeChanged(int)),
                      q,
                      SLOT(sizingModeChanged(int)));
    }

    if (hasSyncFlags(GtDocView::SyncMouseMode)) {
        q->disconnect(m_model,
                      SIGNAL(mouseModeChanged(int)),
                      q,
                      SLOT(mouseModeChanged(int)));
    }
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
    m_heightCache.height(m_document, page,
                         m_rotation, evenPageLeft(), m_scale,
                         height, dualHeight);
}

int GtDocViewPrivate::pageYOffset(int page)
{
    QSize maxSize = m_document->maxPageSize(m_scale, m_rotation);
    QRect border = computeBorder(maxSize);
    double offset = 0;

    if (m_layoutMode != GtDocModel::SinglePage) {
        int e = evenPageLeft();
        heightToPage(page, 0, &offset);
        offset += ((page + e) / 2 + 1) * m_spacing +
                  ((page + e) / 2 ) * (border.top() + border.bottom());
    }
    else {
        heightToPage(page, &offset, 0);
        offset += (page + 1) * m_spacing +
                  page * (border.top() + border.bottom());
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

QRect GtDocViewPrivate::pageExtents(int page, QRect *border)
{
    Q_Q(GtDocView);

    const QSize vs = q->viewport()->size();
    QSize pageSize;
    QRect b, r;
    int x, y;

    /* Get the size of the page */
    pageSize = pageSizeOfView(page);
    b = computeBorder(pageSize);
    r.setWidth(pageSize.width() + b.left() + b.right());
    r.setHeight(pageSize.height() + b.top() + b.bottom());

    if (m_continuous) {
        QSize maxSize = m_document->maxPageSize(m_scale, m_rotation);
        int maxWidth;

        maxWidth = maxSize.width() + b.left() + b.right();

        /* Get the location of the bounding box */
        if (dualPage()) {
            x = m_spacing + ((page % 2 == evenPageLeft()) ? 0 : 1) * (maxWidth + m_spacing);
            x = x + MAX(0, vs.width() - (maxWidth * 2 + m_spacing * 3)) / 2;
            if (page % 2 == evenPageLeft())
                x = x + (maxWidth - pageSize.width() - b.left() - b.right());
        }
        else {
            x = m_spacing;
            x = x + MAX(0, vs.width() - (pageSize.width() + m_spacing * 2)) / 2;
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
            if (otherPage < m_pageCount && (0 <= otherPage)) {
                otherSize = pageSizeOfView (otherPage);

                if (otherSize.width() > pageSize.width())
                    maxSize.setWidth(otherSize.width());

                if (otherSize.height() > pageSize.height())
                    maxSize.setHeight(otherSize.height());
            }

            overallBorder = computeBorder(maxSize);

            /* Find the offsets */
            x = m_spacing;
            y = m_spacing;

            /* Adjust for being the left or right page */
            if (page % 2 == evenPageLeft()) {
                x = x + maxSize.width() - pageSize.width();
            }
            else {
                x = x + (maxSize.width() +
                         overallBorder.left() +
                         overallBorder.right()) + m_spacing;
            }

            y = y + (maxSize.height() - pageSize.height()) / 2;

            /* Adjust for extra allocation */
            x = x + MAX(0, vs.width() - ((maxSize.width() + overallBorder.left() + overallBorder.right()) * 2 + m_spacing * 3)) / 2;
            y = y + MAX(0, vs.height() - (pageSize.height() + m_spacing * 2)) / 2;
        }
        else {
            x = m_spacing;
            y = m_spacing;

            /* Adjust for extra allocation */
            x = x + MAX(0, vs.width() - (pageSize.width() + b.left() + b.right() + m_spacing * 2)) / 2;
            y = y + MAX(0, vs.height() - (pageSize.height() + b.top() + b.bottom() + m_spacing * 2)) / 2;
        }
    }

    r.moveTo(x, y);

    if (border)
        *border = b;

    return r;
}

QSize GtDocViewPrivate::layoutPagesContinuous()
{
    QSize size;

    size.setHeight(pageYOffset(m_pageCount));

    switch (m_sizingMode) {
    case GtDocModel::FitWidth:
    case GtDocModel::BestFit:
        size.setWidth(0);
        break;

    case GtDocModel::FreeSize:
        {
            QSize maxSize = m_document->maxPageSize(m_scale, m_rotation);
            QRect border = computeBorder(maxSize);

            size.setWidth(maxSize.width() + (m_spacing * 2) +
                          border.left() + border.right());
        }
        break;

    default:
        Q_ASSERT(0);
        break;
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

    QSize docSize(m_document->maxPageSize(1.0, m_rotation));
    QRect border(computeBorder(docSize));

    docSize.setWidth(docSize.width() * 2);
    width -= (2 * (border.left() + border.right()) + 3 * m_spacing);
    height -= (border.top() + border.bottom() + 2 * m_spacing - 1);

    QSize viewSize(width - q->verticalScrollBar()->width(), height);

    if (m_sizingMode == GtDocModel::FitWidth) {
        q->setScale(zoomForSizeFitWidth(docSize, viewSize));
    }
    else if (m_sizingMode == GtDocModel::BestFit) {
        q->setScale(zoomForSizeBestFit(docSize, viewSize));
    }
    else {
        Q_ASSERT(0);
    }
}

void GtDocViewPrivate::zoomForSizeContinuous(int width, int height)
{
    Q_Q(GtDocView);

    QSize docSize(m_document->maxPageSize(1.0, m_rotation));
    QRect border(computeBorder(docSize));

    width -= (border.left() + border.right() + 2 * m_spacing);
    height -= (border.top() + border.bottom() + 2 * m_spacing - 1);

    QSize viewSize(width - q->verticalScrollBar()->width(), height);

    if (m_sizingMode == GtDocModel::FitWidth) {
        q->setScale(zoomForSizeFitWidth(docSize, viewSize));
    }
    else if (m_sizingMode == GtDocModel::BestFit) {
        q->setScale(zoomForSizeBestFit(docSize, viewSize));
    }
    else {
        Q_ASSERT(0);
    }
}

void GtDocViewPrivate::zoomForSizeDualPage(int width, int height)
{
    Q_Q(GtDocView);

    /* Find the largest of the two. */
    QSize docSize(pageSizeOfDoc(m_currentPage));
    int otherPage = m_currentPage ^ 1;

    if (otherPage < m_pageCount) {
        QSize docSize2(pageSizeOfDoc(otherPage));

        if (docSize2.width() > docSize.width())
            docSize.setWidth(docSize2.width());

        if (docSize2.height() > docSize.height())
            docSize.setHeight(docSize2.height());
    }

    QRect border(computeBorder(QSize(width, height)));

    docSize.setWidth(docSize.width() * 2);
    width -= ((border.left() + border.right())* 2 + 3 * m_spacing);
    height -= (border.top() + border.bottom() + 2 * m_spacing);

    if (m_sizingMode == GtDocModel::FitWidth) {
        QSize viewSize(width - q->verticalScrollBar()->width(), height);
        q->setScale(zoomForSizeFitWidth(docSize, viewSize));
    }
    else if (m_sizingMode == GtDocModel::BestFit) {
        QSize viewSize(width, height);
        q->setScale(zoomForSizeBestFit(docSize, viewSize));
    }
    else {
        Q_ASSERT(0);
    }
}

void GtDocViewPrivate::zoomForSizeSinglePage(int width, int height)
{
    Q_Q(GtDocView);

    QSize docSize(pageSizeOfDoc(m_currentPage));
    QRect border(computeBorder(QSize(width, height)));

    width -= (border.left() + border.right() + 2 * m_spacing);
    height -= (border.top() + border.bottom() + 2 * m_spacing);

    if (m_sizingMode == GtDocModel::FitWidth) {
        QSize viewSize(width - q->verticalScrollBar()->width(), height);
        q->setScale(zoomForSizeFitWidth(docSize, viewSize));
    }
    else if (m_sizingMode == GtDocModel::BestFit) {
        QSize viewSize(width, height);
        q->setScale(zoomForSizeBestFit(docSize, viewSize));
    }
    else {
        Q_ASSERT(0);
    }
}

void GtDocViewPrivate::fillRegion(QPainter &p, const QRegion &r, const QColor &c)
{
    QVector<QRect> rects = r.rects();
    int count = rects.count();

    for (int i = 0; i < count; ++i)
        p.fillRect(rects[i], c);
}

void GtDocViewPrivate::drawUnderline(QPainter &p, const QVector<QRect> &rs, const QPoint &offset)
{
    QPen oldPen, pen;

    oldPen = p.pen();
    pen.setColor(m_underlineColor);
    pen.setWidthF(m_underlineWidth * m_scale);
    p.setPen(pen);

    p.setCompositionMode(QPainter::CompositionMode_Multiply);

    QVector<QRect>::const_iterator it;
    for (it = rs.begin(); it != rs.end(); ++it) {
        QRect r = *it;
        int lineWidth = m_underlineWidth * m_scale;
        int x1, y1, x2, y2;

        r.translate(offset);

        if (90 == m_rotation) {
            x1 = r.left() + lineWidth;
            y1 = r.top() + lineWidth;
            x2 = x1;
            y2 = r.bottom() - lineWidth;
        }
        else if (180 == m_rotation) {
            x1 = r.left() + lineWidth;
            y1 = r.top() + lineWidth;
            x2 = r.right() - lineWidth;
            y2 = y1;
        }
        else if (270 == m_rotation) {
            x1 = r.right() - lineWidth;
            y1 = r.top() + lineWidth;
            x2 = x1;
            y2 = r.bottom() - lineWidth;
        }
        else {
            x1 = r.left() + lineWidth;
            y1 = r.bottom() - lineWidth;
            x2 = r.right() - lineWidth;
            y2 = y1;
        }

        p.drawLine(x1, y1, x2, y2);
    }

    p.setCompositionMode(QPainter::CompositionMode_SourceOver);
    p.setPen(oldPen);
}

void GtDocViewPrivate::drawPageNote(QPainter &p, GtDocPage *page,
                                    GtDocNote *note, const QPoint &offset)
{
    switch (note->type()) {
    case GtDocNote::Highlight:
        {
            QRegion noteRegion(rangeRegion(note->range(), page));

            noteRegion.translate(offset);
            p.setCompositionMode(QPainter::CompositionMode_Multiply);
            fillRegion(p, noteRegion, m_highlightColor);
            p.setCompositionMode(QPainter::CompositionMode_SourceOver);
        }
        break;

    case GtDocNote::Underline:
        {
            QPoint selText(note->range().intersectedText(page));

            if (selText.y() > selText.x()) {
                QVector<QRect> rects;

                rects = textRects(page, selText.x(), selText.y());
                drawUnderline(p, rects, offset);
            }
        }
        break;

    default:
        qWarning() << "invalid doc note type:" << note->type();
        break;
    }
}

void GtDocViewPrivate::drawPage(QPainter &p, int index, const QRect &pageArea,
                                const QRect &border, const GtDocRange &selRange)
{
    GtDocPage *page = m_document->page(index);
    QRect realArea(pageArea.x() + border.left(),
                   pageArea.y() + border.top(),
                   pageArea.width() - border.left() - border.right(),
                   pageArea.height() - border.top() - border.bottom());
    QPoint offset(pageArea.x() + border.left(),
                  pageArea.y() + border.top());

    // draw border and background
    int levels = border.right() - border.left();
    int x = realArea.x();
    int y = realArea.y();
    int width = realArea.width();
    int height = realArea.height();

    p.setPen(Qt::black);
    p.drawRect(x - 1, y - 1, width + 1, height + 1);

    // draw bottom/right gradient
    int r = m_backColor.red() / (levels + 2) + 6;
    int g = m_backColor.green() / (levels + 2) + 6;
    int b = m_backColor.blue() / (levels + 2) + 6;

    p.translate(x, y);
    for (int i = 0; i < levels; i++) {
        p.setPen(QColor(r * (i+2), g * (i+2), b * (i+2)));
        p.drawLine(i, i + height + 1, i + width + 1, i + height + 1);
        p.drawLine(i + width + 1, i, i + width + 1, i + height);
        p.setPen(m_backColor);
        p.drawLine(-1, i + height + 1, i - 1, i + height + 1);
        p.drawLine(i + width + 1, -1, i + width + 1, i - 1);
    }

    p.translate(-x, -y);

    // draw page contents
    QImage image = m_renderCache->image(index);
    if (image.isNull()) {
        p.fillRect(realArea, m_paperColor);
        return;
    }

    p.drawImage(realArea, image);

    // draw page notes
    if (m_notes) {
        QList<GtDocNote*> notes = m_notes->pageNotes(index);
        QList<GtDocNote*>::iterator it;

        for (it = notes.begin(); it != notes.end(); ++it) {
            drawPageNote(p, page, (*it), offset);
        }
    }

    // highlight selected region
    QRegion selRegion(rangeRegion(selRange, page));
    selRegion.translate(offset);
    fillRegion(p, selRegion, m_selBgColor);
}

QVector<QRect> GtDocViewPrivate::textRects(GtDocPage *page, int begin, int end) const
{
    GtDocTextPointer text(page->text());
    const QRectF *rect = text->rects() + begin;
    QTransform m = pageAreaToView(page);
    QVector<QRect> rects;
    QRectF lineRect;
    QRectF temp;
    QRect real;
    bool lineDone = false;

    Q_ASSERT(end > begin);

    for (int i = begin; i < end; ++i, ++rect) {
        if (!lineRect.isValid()) {
            lineRect = *rect;
            continue;
        }

        const qreal diff = 10.0;
        if (qAbs(lineRect.top() - rect->top()) < diff &&
            qAbs(lineRect.bottom() - rect->bottom()) < diff)
        {
            if (rect->left() < lineRect.left() &&
                lineRect.left() - rect->right() < diff)
            {
                lineRect.setLeft(rect->left());
            }
            else if (rect->right() > lineRect.right() &&
                     rect->left() - lineRect.right() < diff)
            {
                lineRect.setRight(rect->right());
            }
            else {
                lineDone = true;
            }
        }
        else {
            lineDone = true;
        }

        if (lineDone) {
            temp = m.mapRect(lineRect);
            real.setCoords(temp.left(), temp.top(),
                           temp.right(), temp.bottom());
            rects.push_back(real);
            lineRect = *rect;
            lineDone = false;
        }
    }

    temp = m.mapRect(lineRect);
    real.setCoords(temp.left(), temp.top(), temp.right(), temp.bottom());
    rects.push_back(real);
    return rects;
}

QRegion GtDocViewPrivate::rangeRegion(const GtDocRange &range,
                                      GtDocPage *page) const
{
    QRegion region;

    switch (range.type()) {
    case GtDocRange::TextRange:
        {
            QPoint selText(range.intersectedText(page));

            if (selText.y() > selText.x()) {
                QVector<QRect> rects;

                rects = (textRects(page, selText.x(), selText.y()));
                QVector<QRect>::iterator it;
                for (it = rects.begin(); it != rects.end(); ++it)
                    region += *it;
            }
        }
        break;

    case GtDocRange::GeomRange:
        Q_ASSERT(0);
        break;

    default:
        Q_ASSERT(0);
        break;
    }

    return region;
}

int GtDocViewPrivate::pageDistance(int page, const QPoint &point, QPointF *ppoint)
{
    QRect rect(pageExtents(page));
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

QTransform GtDocViewPrivate::pageAreaToView(GtDocPage *page) const
{
    QTransform m;
    qreal w, h;

    page->size(&w, &h);
    w *= m_scale;
    h *= m_scale;

    if (90 == m_rotation) {
        m = m.translate(h, 0);
    }
    else if (180 == m_rotation) {
        m = m.translate(w, h);
    }
    else if (270 == m_rotation) {
        m = m.translate(0, w);
    }
    else {
        Q_ASSERT(m_rotation == 0);
    }

    m = m.scale(m_scale, m_scale);
    return m.rotate(m_rotation);
}

QPoint GtDocViewPrivate::pageViewToView(int index)
{
    Q_Q(GtDocView);

    QRect pageArea, border;
    QPoint scrollPos(q->scrollPoint());

    pageArea = pageExtents(index, &border);
    pageArea.translate(-scrollPos.x(), -scrollPos.y());

    return QPoint(pageArea.x() + border.left(),
                  pageArea.y() + border.top());
}

GtDocPoint GtDocViewPrivate::docPointFromViewPoint(const QPoint &p, bool inside)
{
    Q_Q(GtDocView);

    if (!isDocLoaded())
        return GtDocPoint();

    QPoint point(p + q->scrollPoint());
    QPointF ppoint;
    QPointF bestPoint;
    int bestPage = -1;
    int bestDistance = -1;
    int distance;

    for (int i = m_beginPage; i < m_endPage; ++i) {
        distance = pageDistance(i, point, &ppoint);
        if (-1 == bestPage || distance < bestDistance) {
            bestPage = i;
            bestPoint = ppoint;
            bestDistance = distance;

            // point inside page
            if (0 == bestDistance)
                break;
        }
    }

    if (inside && bestDistance != 0)
        return GtDocPoint();

    GtDocPage *page = m_document->page(bestPage);
    QTransform m = pageAreaToView(page);
    m = m.inverted();

    return GtDocPoint(bestPage, m.map(bestPoint));
}

QPoint GtDocViewPrivate::viewPointFromDocPoint(const GtDocPoint &p)
{
    QPointF point(p.x(), p.y());
    QTransform m = pageAreaToView(m_document->page(p.page()));
    point = m.map(point);

    QPoint offset(pageViewToView(p.page()));
    return QPoint(point.x() + offset.x(), point.y() + offset.y());
}

void GtDocViewPrivate::updateCursor(const QPoint &p)
{
    Q_Q(GtDocView);

    if (GtDocModel::SelectText == m_mouseMode) {
        GtDocPoint docPoint(docPointFromViewPoint(p, true));

        if (docPoint.text(m_document, true) != -1)
            q->setCursor(Qt::IBeamCursor);
        else
            q->setCursor(Qt::ArrowCursor);
    }
}

void GtDocViewPrivate::relayoutPagesLater()
{
    Q_Q(GtDocView);

    if (!m_pendingRelayoutPages) {
        m_pendingRelayoutPages = true;
        QMetaObject::invokeMethod(q, "relayoutPages", Qt::QueuedConnection);
    }
}

void GtDocViewPrivate::repaintDocRange(const GtDocRange &range)
{
    Q_Q(GtDocView);

    if (range.isEmpty())
        return;

    QRegion updateRegion;
    int selBegin = range.begin().page();
    int selEnd = range.end().page() + 1;

    selBegin = MAX(m_beginPage, selBegin);
    selEnd = MIN(m_endPage, selEnd);
    for (int i = selBegin; i < selEnd; ++i) {
        GtDocPage *page = m_document->page(i);
        QRegion region = rangeRegion(range, page);
        QRect pageArea, border;

        pageArea = pageExtents(i, &border);
        region.translate(pageArea.x() + border.left(),
                         pageArea.y() + border.top());
        updateRegion += region;
    }

    QPoint scrollPos(q->scrollPoint());
    updateRegion.translate(-scrollPos.x(), -scrollPos.y());
    q->viewport()->update(updateRegion);
}

void GtDocViewPrivate::repaintOldAndNewSelection(const GtDocRange &oldRange)
{
    Q_Q(GtDocView);

    GtDocRange newRange(q->selectedRange());
    GtDocRange updateRange;

    if (oldRange.isEmpty()) {
        updateRange = newRange;
    }
    else if (newRange.isEmpty()) {
        updateRange = oldRange;
    }
    else {
        GtDocPoint oldBegin(oldRange.begin());
        GtDocPoint oldEnd(oldRange.end());
        GtDocPoint newBegin(newRange.begin());
        GtDocPoint newEnd(newRange.end());

        updateRange.setType(newRange.type());
        if (newBegin == oldBegin) {
            if (newEnd < oldEnd)
                updateRange.setRange(newEnd, oldEnd);
            else
                updateRange.setRange(oldEnd, newEnd);
        }
        else if (newEnd == oldEnd) {
            if (newBegin < oldBegin)
                updateRange.setRange(newBegin, oldBegin);
            else
                updateRange.setRange(oldBegin, newBegin);
        }
        else {
            updateRange.setRange(newBegin < oldBegin ? newBegin : oldBegin,
                                 newEnd > oldEnd ? newEnd : oldEnd);
        }
    }

    repaintDocRange(updateRange);
}

GtDocView::GtDocView(QWidget *parent)
    : QAbstractScrollArea(parent)
    , d_ptr(new GtDocViewPrivate(this))
{
}

GtDocView::~GtDocView()
{
}

GtDocModel* GtDocView::model() const
{
    Q_D(const GtDocView);
    return d->m_model;
}

void GtDocView::setModel(GtDocModel *model)
{
    Q_D(GtDocView);

    if (model == d->m_model)
        return;

    if (d->m_model) {
        disconnect(d->m_model,
                   SIGNAL(documentChanged(GtDocument*)),
                   this,
                   SLOT(documentChanged(GtDocument*)));

        disconnect(d->m_model,
                   SIGNAL(notesChanged(GtDocNotes*)),
                   this,
                   SLOT(notesChanged(GtDocNotes*)));

        disconnect(d->m_model,
                   SIGNAL(destroyed(QObject*)),
                   this,
                   SLOT(modelDestroyed(QObject*)));

        d->disconnectSyncSignals();

        if (d->m_model->parent() == this)
            delete d->m_model;
    }

    GtDocument *newdoc = 0;
    GtDocNotes *newnotes = 0;
    d->m_model = model;
    d->m_renderCache->clear();

    if (d->m_model) {
        newdoc = model->document();
        newnotes = model->notes();
        d->m_rotation = model->rotation();
        d->m_scale = model->scale();
        d->m_continuous = model->continuous();
        d->m_layoutMode = model->layoutMode();
        d->m_sizingMode = model->sizingMode();
        d->m_mouseMode = model->mouseMode();

        connect(d->m_model,
                SIGNAL(documentChanged(GtDocument*)),
                this,
                SLOT(documentChanged(GtDocument*)));

        connect(d->m_model,
                SIGNAL(notesChanged(GtDocNotes*)),
                this,
                SLOT(notesChanged(GtDocNotes*)));

        connect(d->m_model,
                SIGNAL(destroyed(QObject*)),
                this,
                SLOT(modelDestroyed(QObject*)));

        d->connectSyncSignals();
    }

    documentChanged(newdoc);
    notesChanged(newnotes);
}

GtDocView::SyncFlags GtDocView::syncFlags() const
{
    Q_D(const GtDocView);
    return d->m_syncFlags;
}

void GtDocView::setSyncFlags(SyncFlags flags)
{
    Q_D(GtDocView);

    d->disconnectSyncSignals();
    d->m_syncFlags = flags;
    d->connectSyncSignals();
}

QUndoStack* GtDocView::undoStack() const
{
    Q_D(const GtDocView);
    return d->m_undoStack;
}

void GtDocView::setUndoStack(QUndoStack *undoStack)
{
    Q_D(GtDocView);

    if (undoStack == d->m_undoStack)
        return;

    if (d->m_undoStack) {
        disconnect(d->m_undoStack,
                   SIGNAL(destroyed(QObject*)),
                   this,
                   SLOT(undoStackDestroyed(QObject*)));

        if (d->m_undoStack->parent() == this)
            delete d->m_undoStack;
    }

    d->m_undoStack = undoStack;
    if (d->m_undoStack) {
        connect(d->m_undoStack,
                SIGNAL(destroyed(QObject*)),
                this,
                SLOT(undoStackDestroyed(QObject*)));
    }
}

void GtDocView::setRenderThread(QThread *thread)
{
    Q_D(GtDocView);
    d->m_renderCache->moveToThread(thread);
}

void GtDocView::setRenderCacheSize(int size)
{
    Q_D(GtDocView);
    d->m_renderCache->setMaxSize(size);
}

void GtDocView::lockPageUpdate()
{
    Q_D(GtDocView);
    d->m_lockPageUpdate++;
}

void GtDocView::unlockPageUpdate(bool update)
{
    Q_D(GtDocView);

    Q_ASSERT(d->m_lockPageUpdate > 0);

    if (update)
        d->m_lockPageNeedUpdate = true;

    d->m_lockPageUpdate--;
    if (0 == d->m_lockPageUpdate && d->m_lockPageNeedUpdate) {
        QMetaObject::invokeMethod(this, "updateVisiblePages", Qt::QueuedConnection);
        d->m_lockPageNeedUpdate = false;
    }
}

double GtDocView::scale() const
{
    Q_D(const GtDocView);
    return d->m_scale;
}

void GtDocView::setScale(double scale)
{
    Q_D(GtDocView);

    d->m_model->setScale(scale);

    if (!d->hasSyncFlags(SyncScale))
        scaleChanged(d->m_model->scale());
}

int GtDocView::rotation() const
{
    Q_D(const GtDocView);
    return d->m_rotation;
}

void GtDocView::setRotation(int rotation)
{
    Q_D(GtDocView);

    d->m_model->setRotation(rotation);

    if (!d->hasSyncFlags(SyncRotation))
        rotationChanged(d->m_model->rotation());
}

int GtDocView::sizingMode() const
{
    Q_D(const GtDocView);
    return d->m_sizingMode;
}

void GtDocView::setSizingMode(int mode)
{
    Q_D(GtDocView);

    d->m_model->setSizingMode((GtDocModel::SizingMode)mode);

    if (!d->hasSyncFlags(SyncSizingMode))
        sizingModeChanged(d->m_model->sizingMode());
}

bool GtDocView::canZoomIn() const
{
    Q_D(const GtDocView);

    if (d->m_model)
        return d->m_scale < d->m_model->maxScale();

    return false;
}

bool GtDocView::canZoomOut() const
{
    Q_D(const GtDocView);

    if (d->m_model)
        return d->m_scale > d->m_model->minScale();

    return false;
}

QRect GtDocView::pageExtents(int page, QRect *border) const
{
    return d_ptr->pageExtents(page, border);
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

void GtDocView::scrollTo(const QPoint &pos)
{
    scrollTo(pos.x(), pos.y());
}

GtLinkDest GtDocView::scrollDest() const
{
    GtDocPoint dp(d_ptr->docPointFromViewPoint(QPoint(0, 0), false));
    return GtLinkDest(dp.page(), dp.point(), d_ptr->m_scale);
}

void GtDocView::scrollTo(const GtLinkDest &dest)
{
    Q_D(GtDocView);

    if (dest.zoom() > 0) {
        setScale(dest.zoom());
        relayoutPages();
    }

    GtDocPoint dp(dest.page(), dest.point());
    scrollTo(scrollPoint() + d->viewPointFromDocPoint(dp));
}

void GtDocView::select(const GtDocPoint &begin, const GtDocPoint &end)
{
    Q_D(GtDocView);

    GtDocRange oldRange(selectedRange());
    d->m_selectBegin = begin;
    d->m_selectEnd = end;
    d->repaintOldAndNewSelection(oldRange);
}

void GtDocView::deselect()
{
    select(GtDocPoint(), GtDocPoint());
}

GtDocRange GtDocView::selectedRange() const
{
    Q_D(const GtDocView);

    GtDocRange selRange;

    bool selText = (GtDocModel::SelectText == d->m_mouseMode);
    if (selText) {
        d->m_selectBegin.text(d->m_document, true);
        d->m_selectEnd.text(d->m_document, false);
        selRange.setType(GtDocRange::TextRange);
    }
    else {
        selRange.setType(GtDocRange::GeomRange);
    }

    if (d->m_selectBegin < d->m_selectEnd) {
        selRange.setRange(d->m_selectBegin, d->m_selectEnd);
    }
    else {
        selRange.setRange(d->m_selectEnd, d->m_selectBegin);
    }

    if (d->m_selectWordOnDoubleClick) {
        return GtDocRange(selRange.begin().beginOfWord(d->m_document, false),
                          selRange.end().endOfWord(d->m_document, false),
                          selRange.type());
    }

    return selRange;
}

GtDocPoint GtDocView::docPointFromViewPoint(const QPoint &p,
                                            bool inside)
{
    Q_D(GtDocView);

    if (d->m_pendingRelayoutPages)
        relayoutPages();

    return d->docPointFromViewPoint(p, inside);
}

QPoint GtDocView::viewPointFromDocPoint(const GtDocPoint &p)
{
    Q_D(GtDocView);

    if (d->m_pendingRelayoutPages)
        relayoutPages();

    return d->viewPointFromDocPoint(p);
}

void GtDocView::highlight()
{
    Q_D(GtDocView);

    GtDocRange range(selectedRange());
    if (range.isEmpty())
        return;

    GtDocNote *note = new GtDocNote(GtDocNote::Highlight, range);
    QUndoCommand *command = new GtNoteCommand(d->m_model, note);

    command->setText(tr("\"Highlight\""));
    d->m_undoStack->push(command);

    deselect();
}

void GtDocView::underline()
{
    Q_D(GtDocView);

    GtDocRange range(selectedRange());
    if (range.isEmpty())
        return;

    GtDocNote *note = new GtDocNote(GtDocNote::Underline, range);
    QUndoCommand *command = new GtNoteCommand(d->m_model, note);

    command->setText(tr("\"Underline\""));
    d->m_undoStack->push(command);

    deselect();
}

void GtDocView::copy()
{
    Q_D(GtDocView);

    GtDocRange selRange(selectedRange());

    if (selRange.isEmpty())
        return;

    QClipboard *clipboard = QApplication::clipboard();
    switch (selRange.type()) {
    case GtDocRange::TextRange:
        {
            int selBegin = selRange.begin().page();
            int selEnd = selRange.end().page() + 1;
            QString selText;

            for (int i = selBegin; i < selEnd; ++i) {
                GtDocPage *page = d->m_document->page(i);
                QPoint textRange(selRange.intersectedText(page));
                GtDocTextPointer text(page->text());

                selText.append(text->texts() + textRange.x(),
                               textRange.y() - textRange.x());
            }

            clipboard->setText(selText, QClipboard::Clipboard);
        }
        break;

    default:
        Q_ASSERT(0);
        break;
    }
}

void GtDocView::rotateLeft()
{
    Q_D(GtDocView);
    setRotation(d->m_rotation - 90);
}

void GtDocView::rotateRight()
{
    Q_D(GtDocView);
    setRotation(d->m_rotation + 90);
}

void GtDocView::zoomIn()
{
    Q_D(GtDocView);

    double scale = d->m_scale * ZOOM_IN_FACTOR;
    setSizingMode(GtDocModel::FreeSize);
    setScale(scale);
}

void GtDocView::zoomOut()
{
    Q_D(GtDocView);

    double scale = d->m_scale * ZOOM_OUT_FACTOR;
    setSizingMode(GtDocModel::FreeSize);
    setScale(scale);
}

void GtDocView::renderFinished(int page)
{
    Q_D(GtDocView);

    int scrollX = horizontalScrollBar()->value();
    int scrollY = verticalScrollBar()->value();
    QRect pageArea(d->pageExtents(page));

    viewport()->repaint(pageArea.translated(-scrollX, -scrollY));
}

void GtDocView::modelDestroyed(QObject *object)
{
    Q_D(GtDocView);

    if (object == static_cast<QObject *>(d->m_model))
        setModel(0);
}

void GtDocView::undoStackDestroyed(QObject *object)
{
    Q_D(GtDocView);

    if (object == static_cast<QObject *>(d->m_undoStack))
        setUndoStack(0);
}

void GtDocView::documentChanged(GtDocument *document)
{
    Q_D(GtDocView);

    if (document && !document->isLoaded()) {
        connect(document,
                SIGNAL(loaded(GtDocument*)),
                this,
                SLOT(documentLoaded(GtDocument*)));
    }

    d->m_document = document;
    d->m_pageCount = d->isDocLoaded() ? d->m_document->pageCount() : 0;
    d->m_renderCache->clear();
    d->m_selectBegin = GtDocPoint();
    d->m_selectEnd = GtDocPoint();

    if (d->isDocLoaded()) {
        d->m_currentPage = d->m_model->page();
    }

    d->relayoutPagesLater();
}

void GtDocView::documentLoaded(GtDocument *document)
{
    Q_D(GtDocView);

    if (document != d->m_document)
        return;

    if (!d->isDocLoaded())
        return;

    d->m_pageCount = d->m_document->pageCount();
    d->m_currentPage = d->m_model->page();

    d->relayoutPagesLater();
}

void GtDocView::bookmarksChanged(GtBookmarks *bookmarks)
{
    Q_D(GtDocView);

    d->m_bookmarks = bookmarks;

    if (!d->m_pendingRelayoutPages)
        viewport()->update();
}

void GtDocView::notesChanged(GtDocNotes *notes)
{
    Q_D(GtDocView);

    if (d->m_notes) {
        disconnect(d->m_notes, SIGNAL(added(GtDocNote*)),
                   this, SLOT(noteUpdated(GtDocNote*)));
        disconnect(d->m_notes, SIGNAL(removed(GtDocNote*)),
                   this, SLOT(noteUpdated(GtDocNote*)));
    }

    d->m_notes = notes;
    if (d->m_notes) {
        connect(d->m_notes, SIGNAL(added(GtDocNote*)),
                this, SLOT(noteUpdated(GtDocNote*)));
        connect(d->m_notes, SIGNAL(removed(GtDocNote*)),
                this, SLOT(noteUpdated(GtDocNote*)));
    }

    if (!d->m_pendingRelayoutPages)
        viewport()->update();
}

void GtDocView::noteUpdated(GtDocNote *note)
{
    Q_D(GtDocView);
    d->repaintDocRange(note->range());
}

void GtDocView::scaleChanged(double scale)
{
    Q_D(GtDocView);

    if (ABS(d->m_scale - scale) < 0.0000001)
        return;

    d->m_scale = scale;

    if (d->m_sizingMode == GtDocModel::FreeSize)
        d->relayoutPagesLater();
}

void GtDocView::rotationChanged(int rotation)
{
    Q_D(GtDocView);

    d->m_rotation = rotation;
    d->m_renderCache->clear();
    d->relayoutPagesLater();
}

void GtDocView::continuousChanged(bool continuous)
{
    Q_D(GtDocView);

    d->m_continuous = continuous;
    d->relayoutPagesLater();
}

void GtDocView::layoutModeChanged(int mode)
{
    Q_D(GtDocView);

    d->m_layoutMode = static_cast<GtDocModel::LayoutMode>(mode);

    /* FIXME: if we're keeping the pixbuf cache around, we should
     * extend the preload_cache_size to be 2 if dual_page is set.
     */
    d->relayoutPagesLater();
}

void GtDocView::sizingModeChanged(int mode)
{
    Q_D(GtDocView);

    d->m_sizingMode = static_cast<GtDocModel::SizingMode>(mode);

    if (mode != GtDocModel::FreeSize)
        d->relayoutPagesLater();
}

void GtDocView::mouseModeChanged(int mode)
{
    Q_D(GtDocView);

    d->m_mouseMode = static_cast<GtDocModel::MouseMode>(mode);

    if (d->m_selectBegin.isNull() && d->m_selectEnd.isNull())
        return;

    d->m_selectBegin = GtDocPoint();
    d->m_selectEnd = GtDocPoint();
    viewport()->update();
}

void GtDocView::relayoutPages()
{
    Q_D(GtDocView);

    if (!d->m_pendingRelayoutPages)
        return;

    d->m_pendingRelayoutPages = false;

    QSize size;
    if (!d->isDocLoaded()) {
        d->resizeContentArea(size);
        return;
    }

    QScrollBar *hsbar = horizontalScrollBar();
    QScrollBar *vsbar = verticalScrollBar();
    double dx, dy;

    if (hsbar->maximum() > 0)
        dx = (double)hsbar->value() / hsbar->maximum();
    else
        dx = 0;

    if (vsbar->maximum() > 0)
        dy = (double)vsbar->value() / vsbar->maximum();
    else
        dy = 0;

    if (d->m_sizingMode != GtDocModel::FreeSize) {
        int width = viewport()->width();
        int height = viewport()->height();

        if (d->m_continuous && d->dualPage())
            d->zoomForSizeContinuousAndDualPage(width, height);
        else if (d->m_continuous)
            d->zoomForSizeContinuous(width, height);
        else if (d->dualPage())
            d->zoomForSizeDualPage(width, height);
        else
            d->zoomForSizeSinglePage(width, height);
    }

    if (d->m_continuous) {
        if (d->m_layoutMode == GtDocModel::SinglePage)
            size = d->layoutPagesContinuous();
        else
            size = d->layoutPagesContinuousDualPage();
    }
    else {
        if (d->m_layoutMode == GtDocModel::SinglePage)
            size = d->layoutPages();
        else
            size = d->layoutPagesDualPage();
    }

    d->resizeContentArea(size);

    scrollTo(hsbar->maximum() * dx + 0.5, vsbar->maximum() * dy + 0.5);
}

void GtDocView::updateVisiblePages(int newValue)
{
    Q_D(GtDocView);

    if (d->m_lockPageUpdate > 0)
        return;

    int begin = d->m_beginPage;
    int end = d->m_endPage;

    if (!d->isDocLoaded()) {
        d->m_beginPage = -1;
        d->m_endPage = -1;
        d->m_currentPage = -1;
    }
    else if (d->m_continuous) {
        QRect unused, pageArea;
        bool found = false;
        int areaMax = -1, area;
        int bestCurrentPage = -1;
        int i, j = 0;

        const QRect viewportRect(horizontalScrollBar()->value(),
                                 verticalScrollBar()->value(),
                                 viewport()->width(),
                                 viewport()->height());

        for (i = 0; i < d->m_pageCount; ++i) {
            pageArea = d->pageExtents(i);
            unused = viewportRect.intersected(pageArea);

            if (!unused.isEmpty()) {
                area = unused.width() * unused.height();

                if (!found) {
                    areaMax = area;
                    d->m_beginPage = i;
                    found = true;
                    bestCurrentPage = i;
                }

                if (area > areaMax) {
                    bestCurrentPage = (area == areaMax) ? MIN(i, bestCurrentPage) : i;
                    areaMax = area;
                }

                d->m_endPage = i + 1;
                j = 0;
            }
            else if (found && d->m_currentPage < d->m_endPage) {
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

        bestCurrentPage = MAX(bestCurrentPage, d->m_beginPage);

        if (d->m_currentPage != bestCurrentPage) {
            d->m_currentPage = bestCurrentPage;
            d->m_model->setPage(bestCurrentPage);
        }
    }
    else if (d->dualPage()) {
        if (d->m_currentPage % 2 == d->evenPageLeft()) {
            d->m_beginPage = d->m_currentPage;

            if (d->m_currentPage + 1 < d->m_pageCount)
                d->m_endPage = d->m_beginPage + 2;
            else
                d->m_endPage = d->m_beginPage + 1;
        }
        else {
            if (d->m_currentPage < 1)
                d->m_beginPage = d->m_currentPage;
            else
                d->m_beginPage = d->m_currentPage - 1;

            d->m_endPage = d->m_currentPage + 1;
        }
    }
    else {
        d->m_beginPage = d->m_currentPage;
        d->m_endPage = d->m_currentPage + 1;
    }

    if (d->m_beginPage == -1 || d->m_endPage == -1)
        return;

    if (begin != d->m_beginPage || end != d->m_endPage) {
    }

    d->m_renderCache->setPageRange(d->m_beginPage, d->m_endPage, d->m_currentPage);

    if (-1 == newValue)
        viewport()->update();
}

void GtDocView::scrollUp(bool singleStep)
{
    Q_D(GtDocView);

    // if in single page mode and at the top of the screen, go to \ page
    if (d->m_continuous ||
        verticalScrollBar()->value() > verticalScrollBar()->minimum())
    {
        if (singleStep)
            verticalScrollBar()->triggerAction(QScrollBar::SliderSingleStepSub);
        else
            verticalScrollBar()->triggerAction(QScrollBar::SliderPageStepSub);
    }
    else if (d->m_model->page() > 0) {
        Q_ASSERT(0);
    }
}

void GtDocView::scrollDown(bool singleStep)
{
    Q_D(GtDocView);

    // if in single page mode and at the bottom of the screen, go to next page
    if (d->m_continuous ||
        verticalScrollBar()->value() < verticalScrollBar()->maximum())
    {
        if (singleStep)
            verticalScrollBar()->triggerAction(QScrollBar::SliderSingleStepAdd);
        else
            verticalScrollBar()->triggerAction(QScrollBar::SliderPageStepAdd);
    }
    else if (d->m_model->page() < d->m_document->pageCount() - 1) {
        Q_ASSERT(0);
    }
}

void GtDocView::resizeEvent(QResizeEvent *e)
{
    Q_D(GtDocView);

    if (d->m_sizingMode == GtDocModel::FitWidth &&
        d->m_verticalScrollBarVisible && !verticalScrollBar()->isVisible() &&
        qAbs(e->oldSize().height() - e->size().height()) < verticalScrollBar()->width())
    {
        // this saves us from infinite resizing loop because of scrollbars appearing and disappearing
        // see bug 160628 for more info
        // TODO looks are still a bit ugly because things are left uncentered
        // but better a bit ugly than unusable
        d->m_verticalScrollBarVisible = false;
        d->resizeContentArea(e->size());
        return;
    }

    d->m_verticalScrollBarVisible = verticalScrollBar()->isVisible();
    d->relayoutPagesLater();
}

void GtDocView::keyPressEvent(QKeyEvent *e)
{
    e->accept();

    switch (e->key()) {
    case Qt::Key_J:
    case Qt::Key_K:
    case Qt::Key_Down:
    case Qt::Key_PageDown:
    case Qt::Key_Up:
    case Qt::Key_PageUp:
    case Qt::Key_Backspace:
        if (e->key() == Qt::Key_Down
            || e->key() == Qt::Key_PageDown
            || e->key() == Qt::Key_J)
        {
            bool singleStep = e->key() == Qt::Key_Down || e->key() == Qt::Key_J;
            scrollDown(singleStep);
        }
        else {
            bool singleStep = e->key() == Qt::Key_Up || e->key() == Qt::Key_K;
            scrollUp(singleStep);
        }
        break;

    case Qt::Key_Left:
    case Qt::Key_H:
        if (horizontalScrollBar()->maximum() == 0) {
            Q_ASSERT(0);
        }
        else
            horizontalScrollBar()->triggerAction(QScrollBar::SliderSingleStepSub);
        break;

    case Qt::Key_Right:
    case Qt::Key_L:
        if (horizontalScrollBar()->maximum() == 0) {
            Q_ASSERT(0);
        }
        else
            horizontalScrollBar()->triggerAction(QScrollBar::SliderSingleStepAdd);
        break;

    default:
        e->ignore();
        break;
    }
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

    if (!d->isDocLoaded()) {
        QAbstractScrollArea::wheelEvent(e);
        return;
    }

    int delta = e->delta();
    int vScroll = verticalScrollBar()->value();
    Qt::KeyboardModifiers modifiers = e->modifiers();

    e->accept();
    if ((modifiers & Qt::ControlModifier) == Qt::ControlModifier) {
        GtDocPoint docPoint = d->docPointFromViewPoint(e->pos(), false);

        if (delta < 0) {
            if (canZoomOut())
                zoomOut();
            else
                docPoint = GtDocPoint();
        }
        else {
            if (canZoomIn())
                zoomIn();
            else
                docPoint = GtDocPoint();
        }

        // scroll to cursor pos
        if (!docPoint.isNull()) {
            relayoutPages();

            QPoint point(d->viewPointFromDocPoint(docPoint));
            point += scrollPoint();
            point -= e->pos();
            scrollTo(point.x(), point.y());
        }
    }
    else if ((modifiers & Qt::ShiftModifier) == Qt::ShiftModifier) {
    }
    else if (delta <= -120 && !d->m_continuous &&
             vScroll == verticalScrollBar()->maximum())
    {
        // go to next page
        if (d->m_currentPage + 1 < d->m_pageCount) {
            qDebug() << "next page";
        }
    }
    else if (delta >= 120 && !d->m_continuous &&
             vScroll == verticalScrollBar()->minimum())
    {
        // go to prev page
        if (d->m_currentPage > 0) {
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

    // draw page contents
    if (d->isDocLoaded()) {
        QRect pageArea, border, overlap;
        GtDocRange selRange(selectedRange());

        for (int i = d->m_beginPage; i < d->m_endPage; ++i) {
            pageArea = d->pageExtents(i, &border);
            pageArea.translate(-scrollX, -scrollY);
            overlap = contentsRect.intersected(pageArea);

            if (!overlap.isValid())
                continue;

            d->drawPage(p, i, pageArea, border, selRange);
            remainingArea -= pageArea;
        }
    }

    // fill with background color the unpainted area
    d->fillRegion(p, remainingArea, d->m_backColor);
}

void GtDocView::mouseMoveEvent(QMouseEvent *e)
{
    Q_D(GtDocView);

    if (e->buttons() == Qt::LeftButton) {
        if (!d->m_selectBegin.isNull()) {
            GtDocPoint docPoint = d->docPointFromViewPoint(e->pos(), false);
            if (docPoint != d->m_selectEnd) {
                GtDocRange oldRange(selectedRange());
                d->m_selectEnd = docPoint;
                d->repaintOldAndNewSelection(oldRange);
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

    switch (e->buttons()) {
    case Qt::LeftButton:
        if (d->isDocLoaded() && d->m_mouseMode != GtDocModel::BrowseMode) {
            GtDocRange oldRange(selectedRange());
            if (GtDocModel::SelectText == d->m_mouseMode) {
                GtDocPoint docPoint(docPointFromViewPoint(e->pos(), true));

                if (docPoint.text(d->m_document, true) != -1)
                    d->m_selectBegin = docPoint;
                else
                    d->m_selectBegin = GtDocPoint();
            }

            d->m_selectEnd = GtDocPoint();
            d->m_selectWordOnDoubleClick = false;
            d->repaintOldAndNewSelection(oldRange);
        }
        break;

    case Qt::RightButton:
        {
            GtDocRange selRange(selectedRange());
            GtDocPoint docPoint(d->docPointFromViewPoint(e->pos(), true));

            if (!selRange.isEmpty() && !docPoint.isNull()) {
                GtDocPage *page = d->m_document->page(docPoint.page());
                QRegion selRegion(d->rangeRegion(selRange, page));
                QPoint offset(d->pageViewToView(page->index()));

                selRegion.translate(offset);
                if (!selRegion.contains(e->pos())) {
                    GtDocRange oldRange(selectedRange());
                    d->m_selectBegin = GtDocPoint();
                    d->m_selectEnd = GtDocPoint();
                    d->m_selectWordOnDoubleClick = false;
                    d->repaintOldAndNewSelection(oldRange);
                }
            }
        }
        break;

    default:
        break;
    }
}

void GtDocView::mouseReleaseEvent(QMouseEvent *e)
{
    Q_D(GtDocView);

    d->updateCursor(e->pos());
}

void GtDocView::mouseDoubleClickEvent(QMouseEvent *e)
{
    Q_D(GtDocView);

    if (e->button() == Qt::LeftButton) {
        if (d->isDocLoaded() && GtDocModel::SelectText == d->m_mouseMode) {
            GtDocRange oldRange(selectedRange());
            GtDocPoint docPoint(docPointFromViewPoint(e->pos(), true));

            if (docPoint.text(d->m_document, true) != -1)
                d->m_selectBegin = docPoint;
            else
                d->m_selectBegin = GtDocPoint();

            d->m_selectEnd = d->m_selectBegin;
            d->m_selectWordOnDoubleClick = true;
            d->repaintOldAndNewSelection(oldRange);
        }
    }
}

void GtDocView::timerEvent(QTimerEvent *)
{
    qDebug() << ">>>>>>>>>>>>>>>>>>>>>";
}

bool GtDocView::viewportEvent(QEvent *e)
{
    return QAbstractScrollArea::viewportEvent(e);
}

GT_END_NAMESPACE
