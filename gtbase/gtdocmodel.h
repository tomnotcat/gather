/*
 * Copyright (C) 2013 Tom Wong. All rights reserved.
 */
#ifndef __GT_DOC_MODEL_H__
#define __GT_DOC_MODEL_H__

#include "gtcommon.h"
#include <QtCore/qobject.h>

GT_BEGIN_NAMESPACE

class GtDocument;
class GtDocModelPrivate;

class GT_BASE_EXPORT GtDocModel : public QObject
{
    Q_OBJECT

public:
    explicit GtDocModel(QObject *parent = 0);
    explicit GtDocModel(GtDocument *document, QObject *parent = 0);
    ~GtDocModel();

public:
    GtDocument* document() const;
    void setDocument(GtDocument *document);

    int page() const;
    void setPage(int page);

    double scale() const;
    void setScale(double scale);

    double maxScale() const;
    void setMaxScale(double maxScale);

    double minScale() const;
    void setMinScale(double minScale);

    int rotation() const;
    void setRotation(int rotation);

    bool continuous() const;
    void setContinuous(bool continuous);

    enum LayoutMode {
        SinglePage,
        EvenPageLeft,
        OddPageLeft
    };

    LayoutMode layoutMode() const;
    void setLayoutMode(LayoutMode mode);

    enum SizingMode {
        FreeSize,
        BestFit,
        FitWidth
    };

    SizingMode sizingMode() const;
    void setSizingMode(SizingMode mode);

Q_SIGNALS:
    void documentChanged(GtDocument *document);
    void pageChanged(int page);
    void scaleChanged(double scale);
    void rotationChanged(int rotation);
    void continuousChanged(bool continuous);
    void layoutModeChanged(int mode);
    void sizingModeChanged(int mode);

private Q_SLOTS:
    void documentDestroyed(QObject *object);

private:
    QScopedPointer<GtDocModelPrivate> d_ptr;

private:
    Q_DISABLE_COPY(GtDocModel)
    Q_DECLARE_PRIVATE(GtDocModel)
};

GT_END_NAMESPACE

#endif  /* __GT_DOC_MODEL_H__ */
