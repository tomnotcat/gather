/*
 * Copyright (C) 2013 Tom Wong. All rights reserved.
 */
#include "gtdocloader.h"
#include <QtCore/QDebug>
#include <QtCore/QDir>
#include <QtCore/QJsonDocument>

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

int GtDocLoader::registerLoaders(const QString &loaderDir)
{
    QDir dir(loaderDir);

    QStringList filters;
    filters << "*.json";
    dir.setNameFilters(filters);
    dir.setFilter(QDir::Files | QDir::Readable);

    QFile file;
    QJsonDocument metaDoc;
    QFileInfoList list = dir.entryInfoList();
    for (int i = 0; i < list.size(); ++i) {
        QFileInfo fileInfo = list.at(i);

        file.setFileName(fileInfo.fileName());
        if (file.open(QIODevice::ReadOnly)) {
            metaDoc = QJsonDocument::fromJson(file.readAll());
            file.close();
            qDebug() << metaDoc;
        }
    }

    return 0;
}

GtDocument* GtDocLoader::loadDocument(const QString &fileName)
{
    Q_UNUSED(fileName);
    return 0;
}

GT_END_NAMESPACE
