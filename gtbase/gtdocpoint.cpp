/*
 * Copyright (C) 2013 Tom Wong. All rights reserved.
 */
#include "gtdocpoint.h"
#include "gtdocpage.h"
#include <QtCore/QDebug>
#include <QtCore/QPoint>
#include <QtCore/QRect>
#include <math.h>

GT_BEGIN_NAMESPACE

GtDocPoint::GtDocPoint(GtDocPage *page, const QPointF &point)
    : _page(page)
    , _text(-1)
    , _x(point.x())
    , _y(point.y())
{
}

GtDocPoint::GtDocPoint(GtDocPage *page, int text)
    : _page(page)
    , _text(text)
    , _x(-1)
    , _y(-1)
{
    if (text != -1) {
        GtDocTextPointer p(_page->text());
        const QRectF *rects = p->rects();
        int len = p->length();

        Q_ASSERT(len > 0 && text <= len);

        if (text < len) {
            _x = rects[text].left();
            _y = rects[text].top();
        }
        else {
            _x = rects[len - 1].right();
            _y = rects[len - 1].top();
        }
    }
}

bool GtDocPoint::isValid() const
{
    if (_page) {
        qreal w, h;
        _page->size(&w, &h);
        return _x >= 0 && _x <= w && _y >= 0 && _y <= h;
    }

    return false;
}

void GtDocPoint::normalize()
{
    if (_page) {
        qreal w, h;
        _page->size(&w, &h);
        _x = CLAMP(_x, 0, w);
        _y = CLAMP(_y, 0, h);
    }
}

int GtDocPoint::text(bool inside) const
{
    if (_text != -1)
        return _text;

    if (!_page)
        return -1;

    GtDocTextPointer text(_page->text());
    const QRectF *rect = text->rects();
    int i, result = -1;

    if (inside) {
        for (i = 0; i < text->length(); ++i, ++rect) {
            if (_x >= rect->left() && _x < rect->right() &&
                _y >= rect->top() && _y < rect->bottom())
            {
                result = i;
                break;
            }
        }
    }
    else {
        double dist, maxDist = -1;

        for (i = 0; i < text->length(); ++i, ++rect) {
            dist = hypot(_x - rect->x() - rect->width() / 2.0,
                         _y - rect->y() - rect->height() / 2.0);
            if (maxDist < 0 || dist < maxDist) {
                maxDist = dist;
                result = i;
            }
        }
    }

    // check if point is inside right half of the char
    if (result != -1) {
        if (text->texts()[result] != '\n') {
            rect = text->rects() + result;

            if (_x > rect->x() + rect->width() / 2.0)
                ++result;
        }

        *(int*)&_text = result;
    }

    return result;
}

GtDocPoint GtDocPoint::beginOfWord(bool inside) const
{
    int pos = text(inside);
    if (-1 == pos)
        return GtDocPoint();

    GtDocTextPointer text(_page->text());
    const QChar *texts = text->texts();
    int len = text->length();

    if (pos == len)
        --pos;

    if (isSpace(texts[pos])) {
        while (pos && isSpace(texts[pos - 1]))
            --pos;
    }
    else if (isWordSeparator(texts[pos])) {
        while (pos && isWordSeparator(texts[pos - 1]))
            --pos;
    }
    else {
        while (pos &&
               !isSpace(texts[pos - 1]) &&
               !isWordSeparator(texts[pos - 1]))
        {
            --pos;
        }
    }

    return GtDocPoint(_page, pos);
}

GtDocPoint GtDocPoint::endOfWord(bool inside) const
{
    int pos = text(inside);
    if (-1 == pos)
        return GtDocPoint();

    GtDocTextPointer text(_page->text());
    const QChar *texts = text->texts();
    int len = text->length();

    if (pos < len) {
        if (isSpace(texts[pos])) {
            ++pos;
            while (pos < len && isSpace(texts[pos]))
                ++pos;
        }
        else if (isWordSeparator(texts[pos])) {
            ++pos;
            while (pos < len && isWordSeparator(texts[pos]))
                ++pos;
        }
        else {
            ++pos;
            while (pos < len &&
                   !isSpace(texts[pos]) &&
                   !isWordSeparator(texts[pos]))
            {
                ++pos;
            }
        }
    }

    return GtDocPoint(_page, pos);
}

bool operator==(const GtDocPoint &p1, const GtDocPoint &p2)
{
    if (p1._page != p2._page)
        return false;

    if (p1._text != -1 && p2._text != -1)
        return p1._text == p2._text;

    return p1._x == p2._x && p1._y == p2._y;
}

bool operator!=(const GtDocPoint &p1, const GtDocPoint &p2)
{
    if (p1._page != p2._page)
        return true;

    if (p1._text != -1 && p2._text != -1)
        return p1._text != p2._text;

    return p1._x != p2._x || p1._y != p2._y;
}

bool operator>(const GtDocPoint &p1, const GtDocPoint &p2)
{
    int index1 = p1.page() ? p1.page()->index() : -1;
    int index2 = p2.page() ? p2.page()->index() : -1;

    if (index1 > index2)
        return true;

    if (index1 != index2)
        return false;

    if (p1._text != -1 && p2._text != -1)
        return p1._text > p2._text;

    if (p1.y() > p2.y())
        return true;

    if (p1.y() == p2.y() && p1.x() > p2.x())
        return true;

    return false;
}

bool operator<(const GtDocPoint &p1, const GtDocPoint &p2)
{
    int index1 = p1.page() ? p1.page()->index() : -1;
    int index2 = p2.page() ? p2.page()->index() : -1;

    if (index1 < index2)
        return true;

    if (index1 != index2)
        return false;

    if (p1._text != -1 && p2._text != -1)
        return p1._text < p2._text;

    if (p1.y() < p2.y())
        return true;

    if (p1.y() == p2.y() && p1.x() < p2.x())
        return true;

    return false;
}

bool operator>=(const GtDocPoint &p1, const GtDocPoint &p2)
{
    int index1 = p1.page() ? p1.page()->index() : -1;
    int index2 = p2.page() ? p2.page()->index() : -1;

    if (index1 > index2)
        return true;

    if (index1 != index2)
        return false;

    if (p1._text != -1 && p2._text != -1)
        return p1._text >= p2._text;

    if (p1.y() > p2.y())
        return true;

    if (p1.y() == p2.y() && p1.x() >= p2.x())
        return true;

    return false;
}

bool operator<=(const GtDocPoint &p1, const GtDocPoint &p2)
{
    int index1 = p1.page() ? p1.page()->index() : -1;
    int index2 = p2.page() ? p2.page()->index() : -1;

    if (index1 < index2)
        return true;

    if (index1 != index2)
        return false;

    if (p1._text != -1 && p2._text != -1)
        return p1._text <= p2._text;

    if (p1.y() < p2.y())
        return true;

    if (p1.y() == p2.y() && p1.x() <= p2.x())
        return true;

    return false;
}

bool GtDocPoint::isSpace(const QChar &c)
{
    switch (c.unicode()) {
    case QChar::Tabulation:
    case QChar::LineFeed:
    case QChar::CarriageReturn:
    case QChar::Space:
    case QChar::Nbsp:
    case QChar::LineSeparator:
        return true;

    default:
        break;
    }

    return false;
}

bool GtDocPoint::isWordSeparator(const QChar &c)
{
    switch (c.unicode()) {
    case '.':
    case ',':
    case '?':
    case '!':
    case '@':
    case '#':
    case '$':
    case ':':
    case ';':
    case '-':
    case '<':
    case '>':
    case '[':
    case ']':
    case '(':
    case ')':
    case '{':
    case '}':
    case '=':
    case '/':
    case '+':
    case '%':
    case '&':
    case '^':
    case '*':
    case '\'':
    case '"':
    case '`':
    case '~':
    case '|':
        return true;

    default:
        break;
    }

    return false;
}

#ifndef QT_NO_DEBUG_STREAM
QDebug operator<<(QDebug dbg, const GtDocPoint &r) {
    int index = r.page() ? r.page()->index() : -1;
    dbg.nospace() << "GtDocPoint(" << index << ' ' << r.x() << ',' << r.y() << ')';
    return dbg.space();
}
#endif

GT_END_NAMESPACE
