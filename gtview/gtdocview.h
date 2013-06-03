/*
 * Copyright (C) 2013 Tom Wong. All rights reserved.
 */
#ifndef __GT_DOC_VIEW_H__
#define __GT_DOC_VIEW_H__

#include "gtcommon.h"
#include <QtWidgets/qabstractscrollarea.h>

GT_BEGIN_NAMESPACE

class GtDocModel;
class GtDocument;
class GtDocViewPrivate;

class GtDocView : public QAbstractScrollArea
{
    Q_OBJECT

public:
    explicit GtDocView(QWidget *parent = 0);
    explicit GtDocView(GtDocModel *model, QWidget *parent = 0);
    ~GtDocView();

public:
    GtDocModel* model() const;
    void setModel(GtDocModel *model);

    bool canZoomIn() const;
    void zoomIn();

    bool canZoomOut() const;
    void zoomOut();

private Q_SLOTS:
    void modelDestroyed(QObject *object);
    void scrollValueChanged(int value);
    void documentChanged(GtDocument *document);
    void pageChanged(int page);
    void scaleChanged(double scale);
    void rotationChanged(int rotation);
    void continuousChanged(bool continuous);
    void layoutModeChanged(int mode);
    void sizingModeChanged(int mode);
    void relayoutPages();

protected:
    QPoint contentAreaPosition() const;

protected:
    void resizeEvent(QResizeEvent *);

    // mouse / keyboard events
    void keyPressEvent(QKeyEvent *);
    void keyReleaseEvent(QKeyEvent *);
    void inputMethodEvent(QInputMethodEvent *);
    void wheelEvent(QWheelEvent *);

    void paintEvent(QPaintEvent *);

private:
    QScopedPointer<GtDocViewPrivate> d_ptr;

private:
    Q_DISABLE_COPY(GtDocView)
    Q_DECLARE_PRIVATE(GtDocView)
};

GT_END_NAMESPACE

#endif  /* __GT_DOC_VIEW_H__ */
