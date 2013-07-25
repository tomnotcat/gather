/*
 * Copyright (C) 2013 Tom Wong. All rights reserved.
 */
#ifndef __GT_DOC_PAGE_H__
#define __GT_DOC_PAGE_H__

#include "gtobject.h"
#include <QtCore/QObject>
#include <QtCore/QSharedDataPointer>
#include <QtCore/QSize>

class QPaintDevice;

GT_BEGIN_NAMESPACE

class GtAbstractPage;
class GtDocPagePrivate;
class GtDocument;

class GT_BASE_EXPORT GtDocText : public QSharedData, public GtObject
{
public:
    GtDocText(QChar *texts, QRectF *rects, int length);
    GtDocText(const GtDocText &);
    ~GtDocText();

public:
    inline const QChar* texts() const { return m_texts; }
    inline const QRectF* rects() const { return m_rects; }
    inline int length() const { return m_length; }

private:
    GtDocText &operator=(const GtDocText &);

private:
    QChar *m_texts;
    QRectF *m_rects;
    int m_length;
};

typedef QExplicitlySharedDataPointer<GtDocText> GtDocTextPointer;

class GT_BASE_EXPORT GtDocPage : public QObject, public GtObject
{
    Q_OBJECT

public:
    explicit GtDocPage(QObject *parent = 0);
    ~GtDocPage();

public:
    GtDocument* document();
    int index();
    QString label();
    void size(double *width, double *height);
    QSize size(double scale = 1.0, int rotation = 0);
    int length();
    GtDocTextPointer text();
    void paint(QPaintDevice *device, double scale = 1.0, int rotation = 0);

protected:
    friend class GtDocument;
    friend class GtDocumentPrivate;
    GtDocPage(GtDocPagePrivate &dd, QObject *parent);
    QScopedPointer<GtDocPagePrivate> d_ptr;

private:
    Q_DISABLE_COPY(GtDocPage)
    Q_DECLARE_PRIVATE(GtDocPage)
};

GT_END_NAMESPACE

#endif  /* __GT_DOC_PAGE_H__ */
