/*
 * Copyright (C) 2013 Tom Wong. All rights reserved.
 */
#include "gtabstractdocument.h"
#include <QtCore/QString>

GT_BEGIN_NAMESPACE

GtAbstractPage::GtAbstractPage()
{
}

GtAbstractPage::~GtAbstractPage()
{
}

GtAbstractOutline::GtAbstractOutline()
{
}

GtAbstractOutline::~GtAbstractOutline()
{
}

void* GtAbstractOutline::childNode(void *node)
{
    Q_UNUSED(node);
    return 0;
}

void GtAbstractOutline::freeNode(void *node)
{
    Q_UNUSED(node);
}

GtAbstractDocument::GtAbstractDocument()
{
}

GtAbstractDocument::~GtAbstractDocument()
{
}

QString GtAbstractDocument::title()
{
    return QString();
}

GtAbstractOutline* GtAbstractDocument::loadOutline()
{
    return 0;
}

GT_END_NAMESPACE
