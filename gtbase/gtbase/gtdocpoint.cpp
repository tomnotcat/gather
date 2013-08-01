/*
 * Copyright (C) 2013 Tom Wong. All rights reserved.
 */
#include "gtdocpoint.h"
#include "gtdocpage.h"
#include "gtdocument.h"
#include <QtCore/QDebug>
#include <QtCore/QPoint>
#include <QtCore/QRect>
#include <math.h>

GT_BEGIN_NAMESPACE

GtDocPoint::GtDocPoint(int page, const QPointF &point)
    : m_page(page)
    , m_text(-1)
    , m_point(point)
{
}

GtDocPoint::GtDocPoint(GtDocPage *page, int text)
    : m_page(page->index())
    , m_text(text)
    , m_point(-1, -1)
{
    if (text != -1) {
        GtDocTextPointer p(page->text());
        const QRectF *rects = p->rects();
        int len = p->length();

        Q_ASSERT(len > 0 && text <= len);

        if (text < len) {
            m_point.setX(rects[text].left());
            m_point.setY(rects[text].top());
        }
        else {
            m_point.setX(rects[len - 1].right());
            m_point.setY(rects[len - 1].top());
        }
    }
}

int GtDocPoint::text(GtDocument *document, bool inside) const
{
    if (m_text != -1)
        return m_text;

    if (-1 == m_page)
        return -1;

    GtDocPage *page = document->page(m_page);
    GtDocTextPointer text(page->text());
    const QRectF *rect = text->rects();
    int i, result = -1;

    if (inside) {
        for (i = 0; i < text->length(); ++i, ++rect) {
            if (m_point.x() >= rect->left() && m_point.x() < rect->right() &&
                m_point.y() >= rect->top() && m_point.y() < rect->bottom())
            {
                result = i;
                break;
            }
        }
    }
    else {
        double dist, maxDist = -1;

        for (i = 0; i < text->length(); ++i, ++rect) {
            dist = hypot(m_point.x() - rect->x() - rect->width() / 2.0,
                         m_point.y() - rect->y() - rect->height() / 2.0);
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

            if (m_point.x() > rect->x() + rect->width() / 2.0)
                ++result;
        }

        *(int*)&m_text = result;
    }

    return result;
}

GtDocPoint GtDocPoint::beginOfWord(GtDocument *document, bool inside) const
{
    int pos = text(document, inside);
    if (-1 == pos)
        return GtDocPoint();

    GtDocPage *page = document->page(m_page);
    GtDocTextPointer text(page->text());
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

    return GtDocPoint(page, pos);
}

GtDocPoint GtDocPoint::endOfWord(GtDocument *document, bool inside) const
{
    int pos = text(document, inside);
    if (-1 == pos)
        return GtDocPoint();

    GtDocPage *page = document->page(m_page);
    GtDocTextPointer text(page->text());
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

    return GtDocPoint(page, pos);
}

bool operator==(const GtDocPoint &p1, const GtDocPoint &p2)
{
    if (p1.m_page != p2.m_page)
        return false;

    if (p1.m_text != -1 && p2.m_text != -1)
        return p1.m_text == p2.m_text;

    return p1.m_point == p2.m_point;
}

bool operator!=(const GtDocPoint &p1, const GtDocPoint &p2)
{
    if (p1.m_page != p2.m_page)
        return true;

    if (p1.m_text != -1 && p2.m_text != -1)
        return p1.m_text != p2.m_text;

    return p1.m_point != p2.m_point;
}

bool operator>(const GtDocPoint &p1, const GtDocPoint &p2)
{
    int index1 = p1.page();
    int index2 = p2.page();

    if (index1 > index2)
        return true;

    if (index1 != index2)
        return false;

    if (p1.m_text != -1 && p2.m_text != -1)
        return p1.m_text > p2.m_text;

    if (p1.y() > p2.y())
        return true;

    if (p1.y() == p2.y() && p1.x() > p2.x())
        return true;

    return false;
}

bool operator<(const GtDocPoint &p1, const GtDocPoint &p2)
{
    int index1 = p1.page();
    int index2 = p2.page();

    if (index1 < index2)
        return true;

    if (index1 != index2)
        return false;

    if (p1.m_text != -1 && p2.m_text != -1)
        return p1.m_text < p2.m_text;

    if (p1.y() < p2.y())
        return true;

    if (p1.y() == p2.y() && p1.x() < p2.x())
        return true;

    return false;
}

bool operator>=(const GtDocPoint &p1, const GtDocPoint &p2)
{
    int index1 = p1.page();
    int index2 = p2.page();

    if (index1 > index2)
        return true;

    if (index1 != index2)
        return false;

    if (p1.m_text != -1 && p2.m_text != -1)
        return p1.m_text >= p2.m_text;

    if (p1.y() > p2.y())
        return true;

    if (p1.y() == p2.y() && p1.x() >= p2.x())
        return true;

    return false;
}

bool operator<=(const GtDocPoint &p1, const GtDocPoint &p2)
{
    int index1 = p1.page();
    int index2 = p2.page();

    if (index1 < index2)
        return true;

    if (index1 != index2)
        return false;

    if (p1.m_text != -1 && p2.m_text != -1)
        return p1.m_text <= p2.m_text;

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
    dbg.nospace() << "GtDocPoint(" << r.page() << ' ' << r.x() << ',' << r.y() << ')';
    return dbg.space();
}
#endif

GT_END_NAMESPACE
