/*
 * Copyright (C) 2013 Tom Wong. All rights reserved.
 */
#include "gtdocrange.h"
#include "gtdocpage.h"
#include <QtCore/QDebug>

GT_BEGIN_NAMESPACE

QPoint GtDocRange::intersectedText(GtDocPage *page)
{
    if (-1 == _textBegin || -1 == _textEnd) {
        _textBegin = _begin.text(false);
        _textEnd = _end.text(false);

        if (-1 == _textBegin || -1 == _textEnd)
            return QPoint();
    }

    int beginIndex = _begin.page()->index();
    int endIndex = _end.page()->index();
    int index = page->index();

    if (index < beginIndex || index > endIndex)
        return QPoint();

    if (index == beginIndex) {
        if (index == endIndex) {
            if (_textEnd > _textBegin)
                return QPoint(_textBegin, _textEnd);

            return QPoint(_textEnd, _textBegin);
        }

        return QPoint(_textBegin, page->length());
    }

    if (index == endIndex)
        return QPoint(0, _textEnd);

    return QPoint(0, page->length());
}

GT_END_NAMESPACE
