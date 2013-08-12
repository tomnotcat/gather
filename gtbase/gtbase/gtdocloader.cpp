/*
 * Copyright (C) 2013 Tom Wong. All rights reserved.
 */
#include "gtdocloader_p.h"
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
    GtDocLoaderPrivate(GtDocLoader *q, QThread *t);
    ~GtDocLoaderPrivate();

public:
    GtDocument* loadDocument(LoaderInfo &info, const QString &fileName);

    static inline void loadDocument(GtDocument *document)
    {
        document->loadDocument();
    }

public:
    GtDocLoader *q_ptr;
    QList<LoaderInfo> m_infoList;
    QThread *m_thread;
    GtDocLoaderProxy *m_proxy;
};

GtDocLoaderProxy::GtDocLoaderProxy()
{
}

GtDocLoaderProxy::~GtDocLoaderProxy()
{
}

void GtDocLoaderProxy::load(GtDocument *document)
{
    QMutexLocker locker(&m_mutex);

    connect(document,
            SIGNAL(destroyed(QObject*)),
            this,
            SLOT(documentDestroyed(QObject*)));

    m_documents.push_back(document);
    QMetaObject::invokeMethod(this, "loadDocument");
}

void GtDocLoaderProxy::loadDocument()
{
    while (m_documents.size() > 0) {
        GtDocument *document;

        m_mutex.lock();
        document = m_documents.front();
        m_mutex.unlock();

        GtDocLoaderPrivate::loadDocument(document);

        disconnect(document,
                   SIGNAL(destroyed(QObject*)),
                   this,
                   SLOT(documentDestroyed(QObject*)));

        m_mutex.lock();
        m_documents.pop_front();
        m_mutex.unlock();
    }
}

void GtDocLoaderProxy::documentDestroyed(QObject *)
{
    // TODO: document destroyed before load complete
    Q_ASSERT(0);
}

GtDocLoaderPrivate::GtDocLoaderPrivate(GtDocLoader *q, QThread *t)
    : q_ptr(q)
    , m_thread(t)
    , m_proxy(0)
{
    if (t) {
        m_proxy = new GtDocLoaderProxy();
        m_proxy->moveToThread(t);
    }
}

GtDocLoaderPrivate::~GtDocLoaderPrivate()
{
    delete m_proxy;
}

GtDocument* GtDocLoaderPrivate::loadDocument(LoaderInfo &info,
                                             const QString &fileName)
{
    GtDocument *document = NULL;

    if (!info.lib->isLoaded()) {
        if (!info.lib->load()) {
            qWarning() << "load library failed:" << info.lib->errorString();
            return NULL;
        }

        info.load = info.lib->resolve("gather_new_document");
        if (NULL == info.load) {
            qWarning() << "resolve symbol gather_load_document failed:"
                       << info.lib->fileName();
            return NULL;
        }
    }

    if (info.load) {
        QScopedPointer<QFile> file(new QFile(fileName));

        if (file->open(QIODevice::ReadOnly)) {
            GtAbstractDocument *ad = ((GtAbstractDocument* (*)())info.load)();
            Q_ASSERT(ad);

            QFileInfo info(fileName);
            document = new GtDocument(ad);
            file->setParent(document);
            document->d_ptr->setDevice(info.fileName(), file.take());

            if (m_thread)
                m_proxy->load(document);
            else
                document->loadDocument();
        }
        else {
            qWarning() << "open file failed:" << fileName;
        }
    }

    return document;
}

GtDocLoader::GtDocLoader(QThread *thread, QObject *parent)
    : QObject(parent)
    , d_ptr(new GtDocLoaderPrivate(this, thread))
{
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
                        d->m_infoList.push_back(info);

                        count++;

                        qDebug() << "register loader:" << info.info.path;
                    }
                    else {
                        qDebug() << "invalid loader:"
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

    QList<const LoaderInfo *> constInfoList;

    for (int i = 0, count = d->m_infoList.count(); i != count; ++i)
        constInfoList.append(&d->m_infoList.at(i).info);

    return constInfoList;
}

GtDocument* GtDocLoader::loadDocument(const QString &fileName)
{
    Q_D(GtDocLoader);

    GtDocument *document = NULL;
    QFileInfo fileInfo(fileName);
    QString ext = fileInfo.suffix().toLower();

    QList<GtDocLoaderPrivate::LoaderInfo> &infoList = d->m_infoList;

    // By extension
    for (int i = 0, count = infoList.count(); i != count; ++i) {
        if (infoList[i].info.extension == ext) {
            document = d->loadDocument(infoList[i], fileName);
            if (document)
                return document;
        }
    }

    // TODO: By mime type

    return 0;
}

GT_END_NAMESPACE
