/*
 * Copyright (C) 2013 Tom Wong. All rights reserved.
 */
#include "gtdocview.h"
#include "gtdocmodel.h"
#include <QtCore/QDebug>
#include <QtGui/QPaintEvent>
#include <QtWidgets/QScrollBar>

GT_BEGIN_NAMESPACE

class GtDocViewPrivate
{
    Q_DECLARE_PUBLIC(GtDocView)

public:
    GtDocViewPrivate();
    ~GtDocViewPrivate();

public:
    void setupView(GtDocView *parent, GtDocModel *model);

private:
    GtDocView *q_ptr;
    GtDocModel *model;
};

GtDocViewPrivate::GtDocViewPrivate()
    : q_ptr(NULL)
    , model(NULL)
{
}

GtDocViewPrivate::~GtDocViewPrivate()
{
}

void GtDocViewPrivate::setupView(GtDocView *parent, GtDocModel *model)
{
    Q_ASSERT(NULL == q_ptr);

    q_ptr = parent;
    this->model = model;

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

GtDocView::GtDocView(QWidget *parent)
    : QAbstractScrollArea(parent)
    , d_ptr(new GtDocViewPrivate())
{
    d_ptr->setupView(this, NULL);
}

GtDocView::GtDocView(GtDocModel *model, QWidget *parent)
    : QAbstractScrollArea(parent)
    , d_ptr(new GtDocViewPrivate())
{
    d_ptr->setupView(this, model);
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
                   SIGNAL(destroyed(QObject*)),
                   this,
                   SLOT(modelDestroyed(QObject*)));
    }

    d->model = model;
    if (d->model) {
        connect(d->model,
                SIGNAL(destroyed(QObject*)),
                this,
                SLOT(modelDestroyed(QObject*)));
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

void GtDocView::scrollValueChanged(int value)
{
    Q_UNUSED(value);
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

void GtDocView::wheelEvent(QWheelEvent *)
{
    qDebug() << "wheel";
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
