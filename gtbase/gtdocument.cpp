/*
 * Copyright (C) 2013 Tom Wong. All rights reserved.
 */
#include "gtdocument_p.h"
#include <QtCore/QDebug>

GT_BEGIN_NAMESPACE

GtDocumentPrivate::GtDocumentPrivate()
    : uniformWidth(0)
    , uniformHeight(0)
    , maxWidth(0)
    , maxHeight(0)
    , minWidth(0)
    , minHeight(0)
    , uniform(false)
    , initialized(false)
{
}

GtDocumentPrivate::~GtDocumentPrivate()
{
}

void GtDocumentPrivate::initialize()
{
    Q_ASSERT(!initialized);

    /*
    count = ctk_document_count_pages (self);
    120     priv->pages = g_ptr_array_new_full (count, _doc_page_destroy);
    121     g_ptr_array_set_size (priv->pages, count);
    122
            123     priv->uniform = TRUE;
    124     for (i = 0; i < count; ++i) {
        125         page = ctk_document_get_page (self, i);
        126         ctk_doc_page_get_size (page, &page_width, &page_height);
        127
                128         if (i == 0) {
            129             priv->uniform_width = page_width;
            130             priv->uniform_height = page_height;
            131             priv->max_width = priv->uniform_width;
            132             priv->max_height = priv->uniform_height;
            133             priv->min_width = priv->uniform_width;
            134             priv->min_height = priv->uniform_height;
            135         }
        else if (priv->uniform &&
                 137                  (priv->uniform_width != page_width ||
                                       138                   priv->uniform_height != page_height))
            139         {
                140             priv->uniform = FALSE;
                141         }
        142
                143         if (!priv->uniform) {
            144             if (page_width > priv->max_width)
                145                 priv->max_width = page_width;
            146
                    147             if (page_width < priv->min_width)
                148                 priv->min_width = page_width;
            149
                    150             if (page_height > priv->max_height)
                151                 priv->max_height = page_height;
            152
                    153             if (page_height < priv->min_height)
                154                 priv->min_height = page_height;
            155         }
        156     }
    */
    initialized = true;
}

GtDocument::GtDocument(QObject *parent)
    : QObject(parent)
    , d_ptr(new GtDocumentPrivate())
{
    d_ptr->q_ptr = this;
}

GtDocument::GtDocument(GtDocumentPrivate &dd, QObject *parent)
    : QObject(parent)
    , d_ptr(&dd)
{
    d_ptr->q_ptr = this;
}

GtDocument::~GtDocument()
{
}

bool GtDocument::getUniformPageSize(double *width, double *height)
{
    Q_D(GtDocument);

    Q_ASSERT(d->initialized);

    if (d->uniform) {
        if (width)
            *width = d->uniformWidth;

        if (height)
            *height = d->uniformHeight;
    }

    return d->uniform;
}

void GtDocument::getMaxPageSize(double *width, double *height)
{
    Q_D(GtDocument);

    Q_ASSERT(d->initialized);

    if (width)
        *width = d->maxWidth;

    if (height)
        *height = d->maxHeight;
}

void GtDocument::getMinPageSize(double *width, double *height)
{
    Q_D(GtDocument);

    Q_ASSERT(d->initialized);

    if (width)
        *width = d->minWidth;

    if (height)
        *height = d->minHeight;
}

GT_END_NAMESPACE
