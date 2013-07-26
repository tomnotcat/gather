/*
 * Copyright (C) 2013 Tom Wong. All rights reserved.
 */
#ifndef __GT_DOC_VIEW_H__
#define __GT_DOC_VIEW_H__

#include "gtobject.h"
#include "gtdocrange.h"
#include <QtWidgets/qabstractscrollarea.h>

GT_BEGIN_NAMESPACE

class GtDocModel;
class GtBookmarks;
class GtDocNotes;
class GtDocument;
class GtDocViewPrivate;

class GT_VIEW_EXPORT GtDocView : public QAbstractScrollArea, public GtObject
{
    Q_OBJECT

public:
    explicit GtDocView(QWidget *parent = 0);
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

    GtDocRange selectedRange() const;
    GtDocPoint docPointFromViewPoint(const QPoint &p, bool inside);
    QPoint viewPointFromDocPoint(const GtDocPoint &p);

public Q_SLOTS:
    void copy();
    void highlight();

private Q_SLOTS:
    void renderFinished(int page);
    void modelDestroyed(QObject *object);
    void documentChanged(GtDocument *document);
    void documentLoaded(GtDocument *document);
    void bookmarksChanged(GtBookmarks *bookmarks);
    void notesChanged(GtDocNotes *notes);
    void pageChanged(int page);
    void scaleChanged(double scale);
    void rotationChanged(int rotation);
    void continuousChanged(bool continuous);
    void layoutModeChanged(int mode);
    void sizingModeChanged(int mode);
    void mouseModeChanged(int mode);
    void relayoutPages();
    void updateVisiblePages(int newValue = -1);
    void scrollUp(bool singleStep = false);
    void scrollDown(bool singleStep = false);

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
    void timerEvent(QTimerEvent *);

    bool viewportEvent(QEvent *e);

private:
    QScopedPointer<GtDocViewPrivate> d_ptr;

private:
    Q_DISABLE_COPY(GtDocView)
    Q_DECLARE_PRIVATE(GtDocView)
};

GT_END_NAMESPACE

#endif  /* __GT_DOC_VIEW_H__ */
