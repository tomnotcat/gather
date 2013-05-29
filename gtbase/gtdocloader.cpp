/*
 * Copyright (C) 2013 Tom Wong. All rights reserved.
 */
#include "gtdocloader.h"

GT_BEGIN_NAMESPACE

class GtDocLoaderPrivate
{
    Q_DECLARE_PUBLIC(GtDocLoader)

public:
    GtDocLoaderPrivate();
    ~GtDocLoaderPrivate();

protected:
    GtDocLoader *q_ptr;
};

GtDocLoaderPrivate::GtDocLoaderPrivate()
{
}

GtDocLoaderPrivate::~GtDocLoaderPrivate()
{
}

GtDocLoader::GtDocLoader(QObject *parent)
    : QObject(parent)
    , d_ptr(new GtDocLoaderPrivate())
{
    d_ptr->q_ptr = this;
}

GtDocLoader::~GtDocLoader()
{
}

GtDocument* GtDocLoader::load(const QString &fileName)
{
    Q_UNUSED(fileName);
    return 0;
}

GT_END_NAMESPACE
