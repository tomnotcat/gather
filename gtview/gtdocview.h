/*
 * Copyright (C) 2013 Tom Wong. All rights reserved.
 */
#ifndef __GT_DOC_VIEW_H__
#define __GT_DOC_VIEW_H__

#include "gtobject.h"
#include <QtWidgets/qabstractscrollarea.h>

GT_BEGIN_NAMESPACE

class GtDocModel;
class GtDocument;
class GtDocViewPrivate;

class GtDocView : public QAbstractScrollArea, public GtObject
{
    Q_OBJECT

public:
    explicit GtDocView(QWidget *parent = 0);
    explicit GtDocView(GtDocModel *model, QWidget *parent = 0);
    ~GtDocView();

public:
    GtDocModel* model() const;
    void setModel(GtDocModel *model);

    void setRenderThread(QThread *thread);
    void setRenderCacheSize(int size);

    void lockPageUpdate();
    void unlockPageUpdate(bool update=true);

    bool canZoomIn() const;
    void zoomIn();

    bool canZoomOut() const;
    void zoomOut();

    QRect pageExtents(int page, QRect *border = 0) const;

    QPoint scrollPoint() const;
    void scrollTo(int x, int y);

private Q_SLOTS:
    void renderFinished(int page);
    void modelDestroyed(QObject *object);
    void documentChanged(GtDocument *document);
    void pageChanged(int page);
    void scaleChanged(double scale);
    void rotationChanged(int rotation);
    void continuousChanged(bool continuous);
    void layoutModeChanged(int mode);
    void sizingModeChanged(int mode);
    void mouseModeChanged(int mode);
    void relayoutPages();
    void updateVisiblePages(int newValue = -1);

protected:
    void resizeEvent(QResizeEvent *);

    // mouse / keyboard events
    void keyPressEvent(QKeyEvent *);
    void keyReleaseEvent(QKeyEvent *);
    void inputMethodEvent(QInputMethodEvent *);
    void wheelEvent(QWheelEvent *);

    void paintEvent(QPaintEvent *);
    void mouseMoveEvent(QMouseEvent *);
    void mousePressEvent(QMouseEvent *);
    void mouseReleaseEvent(QMouseEvent *);
    void mouseDoubleClickEvent(QMouseEvent *);

    bool viewportEvent(QEvent *e);

private:
    QScopedPointer<GtDocViewPrivate> d_ptr;

private:
    Q_DISABLE_COPY(GtDocView)
    Q_DECLARE_PRIVATE(GtDocView)
};

GT_END_NAMESPACE

#endif  /* __GT_DOC_VIEW_H__ */
