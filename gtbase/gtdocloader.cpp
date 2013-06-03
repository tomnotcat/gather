/*
 * Copyright (C) 2013 Tom Wong. All rights reserved.
 */
#include "gtdocloader.h"
#include "gtdocument_p.h"
#include <QtCore/QDebug>
#include <QtCore/QDir>
#include <QtCore/QLibrary>
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>

GT_BEGIN_NAMESPACE

class GtDocLoaderPrivate
{
    Q_DECLARE_PUBLIC(GtDocLoader)

public:
    class LoaderInfo
    {
    public:
        GtDocLoader::LoaderInfo info;
        QLibrary *lib;
        QFunctionPointer load;
    };

public:
    GtDocLoaderPrivate();
    ~GtDocLoaderPrivate();

public:
    GtDocument* loadDocument(LoaderInfo &info, const QString &fileName);

public:
    GtDocLoader *q_ptr;
    QList<LoaderInfo> infoList;
};

GtDocLoaderPrivate::GtDocLoaderPrivate()
{
}

GtDocLoaderPrivate::~GtDocLoaderPrivate()
{
}

GtDocument* GtDocLoaderPrivate::loadDocument(LoaderInfo &info,
                                             const QString &fileName)
{
    GtDocument *document = NULL;

    if (!info.lib->isLoaded()) {
        if (!info.lib->load()) {
            qWarning() << "load library failed:" << info.lib->fileName();
            return NULL;
        }

        info.load = info.lib->resolve("gather_load_document");
        if (NULL == info.load) {
            qWarning() << "resolve symbol gather_load_document failed:"
                       << info.lib->fileName();
            return NULL;
        }
    }

    if (info.load) {
        QFile file(fileName);

        if (file.open(QIODevice::ReadOnly)) {
            document = ((GtDocument* (*)(QIODevice*))info.load)(&file);
            if (document)
                document->d_ptr->initialize();

            file.close();
        }
        else {
            qWarning() << "open file failed:" << fileName;
        }
    }

    return document;
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
    Q_D(GtDocLoader);

    int count = 0;
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

        file.setFileName(fileInfo.filePath());
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QJsonParseError error;
            metaDoc = QJsonDocument::fromJson(file.readAll(), &error);
            file.close();

            if (error.error) {
                qDebug() << "parsing"
                         << fileInfo.filePath()
                         << "error:"
                         << error.errorString();
            }
            else {
                if (metaDoc.isObject()) {
                    QJsonObject obj = metaDoc.object();
                    QJsonValue modVal;
                    QJsonValue descVal;
                    QJsonValue mimeVal;
                    QJsonValue extVal;

                    modVal = obj.value("module");
                    descVal = obj.value("description");
                    mimeVal = obj.value("mimetype");
                    extVal = obj.value("extension");

                    if (modVal.isString() &&
                        descVal.isString() &&
                        (mimeVal.isString() ||
                         extVal.isString()))
                    {
                        GtDocLoaderPrivate::LoaderInfo info;

                        info.info.module = modVal.toString();
                        info.info.description = descVal.toString();

                        if (mimeVal.isString())
                            info.info.mimetype = mimeVal.toString().toLower();

                        if (extVal.isString())
                            info.info.extension = extVal.toString().toLower();

                        info.info.path = fileInfo.filePath();

                        QString libPath(fileInfo.path() + "/" + info.info.module);
                        info.lib = new QLibrary(libPath, this);
                        d->infoList.push_back(info);

                        count++;

                        qDebug() << "add loader info:" << info.info.path;
                    }
                    else {
                        qDebug() << "invalid loader info:"
                                 << modVal << descVal << mimeVal << extVal;
                    }
                }
            }
        }
    }

    return count;
}

QList<const GtDocLoader::LoaderInfo *> GtDocLoader::loaderInfos()
{
    Q_D(GtDocLoader);

    QList<GtDocLoaderPrivate::LoaderInfo> *infoList = &d->infoList;
    QList<const LoaderInfo *> constInfoList;

    for (int i = 0, count = infoList->count(); i != count; ++i)
        constInfoList.append(&infoList->at(i).info);

    return constInfoList;
}

GtDocument* GtDocLoader::loadDocument(const QString &fileName)
{
    Q_D(GtDocLoader);

    GtDocument *document = NULL;
    QFileInfo fileInfo(fileName);
    QString ext = fileInfo.suffix().toLower();

    QList<GtDocLoaderPrivate::LoaderInfo> &infoList = d->infoList;

    // By extension
    for (int i = 0, count = infoList.count(); i != count; ++i) {
        if (infoList[i].info.extension == ext) {
            document = d->loadDocument(infoList[i], fileName);
            if (document)
                return document;
        }
    }

    // By mime type
    Q_ASSERT(0);

    return 0;
}

GT_END_NAMESPACE
