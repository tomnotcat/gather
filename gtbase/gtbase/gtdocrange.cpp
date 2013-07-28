/*
 * Copyright (C) 2013 Tom Wong. All rights reserved.
 */
#include "gtdocrange.h"
#include "gtdocpage.h"
#include <QtCore/QDebug>

GT_BEGIN_NAMESPACE

QPoint GtDocRange::intersectedText(GtDocPage *page) const
{
    int beginText = _begin.text(false);
    int endText = _end.text(false);

    if (-1 == beginText || -1 == endText)
        return QPoint();

    int beginIndex = _begin.page()->index();
    int endIndex = _end.page()->index();
    int index = page->index();

    if (index < beginIndex || index > endIndex)
        return QPoint();

    if (index == beginIndex) {
        if (index == endIndex) {
            if (endText > beginText)
                return QPoint(beginText, endText);

            return QPoint(endText, beginText);
        }

        return QPoint(beginText, page->length());
    }

    if (index == endIndex)
        return QPoint(0, endText);

    return QPoint(0, page->length());
}

GT_END_NAMESPACE
