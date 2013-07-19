/*
 * Copyright (C) 2013 Tom Wong. All rights reserved.
 */
#include "pdfdocument.h"
#include "pdfoutline.h"
#include "pdfpage.h"
#include <QtCore/QDebug>
#include <QtCore/QMutex>
#include <QtCore/QThread>

GT_BEGIN_NAMESPACE

class PdfDocument::LabelRange
{
public:
    LabelRange(pdf_obj *d, int b);

public:
    QString prefix;
    int base;
    int first;
    int length;

    enum NumberStyle {
        None,
        Arabic,
        LowercaseRoman,
        UppercaseRoman,
        UppercaseLatin,
        LowercaseLatin
    } style;
};

PdfDocument::LabelRange::LabelRange(pdf_obj *d, int b)
    : base(b)
{
    style = None;

    pdf_obj *o = pdf_dict_gets(d, "S");
    if (pdf_is_name(o)) {
        const char *name = pdf_to_name(o);

        if (!strcmp(name, "D"))
            style = Arabic;
        else if (!strcmp(name, "R"))
            style = UppercaseRoman;
        else if (!strcmp(name, "r"))
            style = LowercaseRoman;
        else if (!strcmp(name, "A"))
            style = UppercaseLatin;
        else if (!strcmp(name, "a"))
            style = LowercaseLatin;
    }

    o = pdf_dict_gets(d, "P");
    if (pdf_is_string(o))
        prefix = QString((QChar *)pdf_to_str_buf(o), pdf_to_str_len(o));

    o = pdf_dict_gets(d, "St");
    if (pdf_is_int(o))
        first = pdf_to_int(o);
    else
        first = 1;
}

PdfDocument::PdfDocument()
    : _context(0)
    , document(0)
{
}

PdfDocument::~PdfDocument()
{
    qDeleteAll(labelRanges);

    if (document) {
        fz_close_document(document);
        document = NULL;
    }

    if (_context) {
        fz_free_context(_context);
        _context = 0;
    }
}

bool PdfDocument::load(QIODevice *device)
{
    fz_stream *stream;

    _context = fz_new_context(NULL, NULL, FZ_STORE_DEFAULT);
    stream = fz_new_stream(_context, device,
                           PdfDocument::readPdfStream,
                           PdfDocument::closePdfStream);
    stream->seek = PdfDocument::seekPdfStream;

    fz_try(_context) {
        document = fz_open_document_with_stream(_context, "pdf", stream);
    }
    fz_catch(_context) {
        if (document) {
            fz_close_document(document);
            document = 0;
        }
    }

    fz_close(stream);

    // parse page labels
    pdf_document *xref = (pdf_document *)document;
    pdf_obj *catalog;
    pdf_obj *pageLabels;

    catalog = pdf_dict_gets(xref->trailer, "Root");
    pageLabels = pdf_dict_gets(catalog, "PageLabels");

    if (pdf_is_dict(pageLabels)) {
        parseLabels(pageLabels);

        int numPages = fz_count_pages(document);
        LabelRange *range;
        for (int i = 0; i < labelRanges.size(); ++i) {
            range = labelRanges[i];

            if (i + 1 < labelRanges.size()) {
                LabelRange *next = labelRanges[i + 1];
                range->length = next->base - range->base;
            }
            else {
                range->length = numPages - range->base;
            }

            if (range->length < 0)
                range->length = 0;
        }
    }

    return (document != 0);
}

QString PdfDocument::title()
{
    /* TODO: some PDF document's title is meaningless
    pdf_document *xref = (pdf_document *)document;
    pdf_obj *info = pdf_dict_gets(xref->trailer, "Info");
    pdf_obj *obj = pdf_dict_gets(info, "Title");

    if (obj) {
        QString title(pdf_to_str_len(obj) + 1, 0);
        pdf_to_ucs2_buf((unsigned short *)title.data(), obj);
        return title;
    }
    */
    return QString();
}

int PdfDocument::countPages()
{
    return fz_count_pages(document);
}

GtAbstractPage* PdfDocument::loadPage(int index)
{
    fz_page *page = fz_load_page(document, index);
    if (0 == page)
        return 0;

    QString label(indexToLabel(index));
    return new PdfPage(_context, document, page, label);
}

GtAbstractOutline* PdfDocument::loadOutline()
{
    fz_outline *outline = fz_load_outline(document);
    return new PdfOutline(_context, outline);
}

void PdfDocument::parseLabels(pdf_obj *tree)
{
    pdf_obj *nums = pdf_dict_gets(tree, "Nums");
    if (pdf_is_array(nums)) {
        int len = pdf_array_len(nums);
        for (int i = 0; i < len; i += 2) {
            pdf_obj *a = pdf_array_get(nums, i);
            if (!pdf_is_int(a))
                continue;

            int b = pdf_to_int(a);
            a = pdf_array_get(nums, i + 1);
            if (!pdf_is_dict(a))
                continue;

            labelRanges.append(new LabelRange(a, b));
        }
    }

    pdf_obj *kids = pdf_dict_gets(tree, "Kids");
    if (pdf_is_array(kids)) {
        int len = pdf_array_len(kids);
        for (int i = 0; i < len; ++i) {
            pdf_obj *a = pdf_array_get(kids, i);

            if (pdf_is_dict(a))
                parseLabels(a);
        }
    }
}

QString PdfDocument::indexToLabel(int index)
{
    int base = 0;
    LabelRange *range = 0;

    for (int i = 0; i < labelRanges.size(); ++i) {
        range = labelRanges[i];
        if (base <= index && index < base + range->length)
            break;

        base += range->length;
    }

    if (!range)
        return QString();

    int number = index - base + range->first;
    QString numberString;

    switch (range->style) {
    case LabelRange::Arabic:
        numberString = QString::number(number);
        break;

    case LabelRange::LowercaseRoman:
        numberString = toRoman(number, false);
        break;

    case LabelRange::UppercaseRoman:
        numberString = toRoman(number, true);
        break;

    case LabelRange::LowercaseLatin:
        numberString = toLatin(number, false);
        break;

    case LabelRange::UppercaseLatin:
        numberString = toLatin(number, true);
        break;

    case LabelRange::None:
        break;
    }

    return range->prefix + numberString;
}

int PdfDocument::readPdfStream(fz_stream *stm, unsigned char *buf, int len)
{
    QIODevice *device = static_cast<QIODevice*>(stm->state);
    return device->read((char*)buf, len);
}

void PdfDocument::seekPdfStream(fz_stream *stm, int offset, int whence)
{
    QIODevice *device = static_cast<QIODevice*>(stm->state);
    qint64 pos = 0;

    switch (whence) {
    case SEEK_SET:
        break;

    case SEEK_CUR:
        pos = device->pos();
        break;

    case SEEK_END:
        pos = device->size();
        break;

    default:
        Q_ASSERT(0);
    }

    pos += offset;

    if (device->seek(pos)) {
        stm->pos = pos;
        stm->rp = stm->bp;
        stm->wp = stm->bp;
    }
    else {
        qWarning() << "pdf seek error:" << whence << offset;
    }
}

void PdfDocument::closePdfStream(fz_context*, void *state)
{
    QIODevice *device = static_cast<QIODevice*>(state);
    device->close();
}

QString PdfDocument::toRoman(int number, bool uppercase)
{
    static const QChar uppercaseNumerals[] = {'I', 'V', 'X', 'L', 'C', 'D', 'M'};
    static const QChar lowercaseNumerals[] = {'i', 'v', 'x', 'l', 'c', 'd', 'm'};
    QString str;
    int divisor;
    int i, j, k;
    const QChar *wh;

    if (uppercase)
        wh = uppercaseNumerals;
    else
        wh = lowercaseNumerals;

    divisor = 1000;
    for (k = 3; k >= 0; k--) {
        i = number / divisor;
        number = number % divisor;

        switch (i) {
        case 0:
            break;

        case 5:
            str.append(wh[2 * k + 1]);
            break;

        case 9:
            str.append(wh[2 * k + 0]);
            str.append(wh[ 2 * k + 2]);
            break;

        case 4:
            str.append(wh[2 * k + 0]);
            str.append(wh[2 * k + 1]);
            break;

        default:
            if (i > 5) {
                str.append(wh[2 * k + 1]);
                i -= 5;
            }

            for (j = 0; j < i; j++)
                str.append(wh[2 * k + 0]);
        }

        divisor = divisor / 10;
    }

    return str;
}

QString PdfDocument::toLatin(int number, bool uppercase)
{
    QString str;
    QChar letter;
    char base;
    int i, count;

    if (uppercase)
        base = 'A';
    else
        base = 'a';

    count = (number - 1) / 26 + 1;
    letter = base + (number - 1) % 26;

    for (i = 0; i < count; i++)
        str.append(letter);

    return str;
}

GT_DEFINE_DOCUMENT_LOADER(PdfDocument())

GT_END_NAMESPACE
