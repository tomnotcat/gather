/*
 * Copyright (C) 2013 Tom Wong. All rights reserved.
 */
#include "gtdocmanager.h"
#include "gtapplication.h"
#include "gtbookmarks.h"
#include "gtdocloader.h"
#include "gtdocmeta.h"
#include "gtdocmodel.h"
#include "gtdocnotes.h"
#include "gtdocument.h"
#include <QtCore/QDebug>
#include <QtSql/QSqlError>
#include <QtSql/QSqlQuery>
#include <QtWidgets/QUndoStack>

GT_BEGIN_NAMESPACE

class GtDocManagerPrivate
{
    Q_DECLARE_PUBLIC(GtDocManager)

public:
    GtDocManagerPrivate(GtDocManager *q);
    ~GtDocManagerPrivate();

public:
    int clearDocMetas();
    int clearDocuments();

protected:
    GtDocManager *q_ptr;
    GtDocLoader *m_docLoader;
    QThread *m_docThread;
    QHash<GtDocument*, GtDocModel*> m_loadingDocs;
    QHash<GtDocModel*, QUndoStack*> m_undoStatcks;
    QHash<QString, GtDocMeta*> m_docMetas;
    QHash<QString, GtDocModel*> m_docModels;
    QSqlDatabase m_docDatabase;
};

GtDocManagerPrivate::GtDocManagerPrivate(GtDocManager *q)
    : q_ptr(q)
    , m_docThread(0)
{
    m_docLoader = new GtDocLoader(q);
    m_docDatabase = GtApplication::instance()->documentDatabase();
}

GtDocManagerPrivate::~GtDocManagerPrivate()
{
    if (clearDocuments() > 0)
        clearDocMetas();

    Q_ASSERT(m_undoStatcks.size() == 0);
    Q_ASSERT(m_loadingDocs.size() == 0);
    Q_ASSERT(m_docModels.size() == 0);
    Q_ASSERT(m_docMetas.size() == 0);
}

int GtDocManagerPrivate::clearDocMetas()
{
    // clear up any unreferenced doc metas
    QHash<QString, GtDocMeta*>::iterator it;
    int count = 0;

    for (it = m_docMetas.begin(); it != m_docMetas.end();) {
        if (it.value()->ref.load() <= 1) {
            delete it.value();
            it = m_docMetas.erase(it);
            ++count;
        }
        else {
            ++it;
        }
    }

    return count;
}

int GtDocManagerPrivate::clearDocuments()
{
    // clear up any unreferenced documents
    QHash<QString, GtDocModel*>::iterator it;
    QHash<GtDocModel*, QUndoStack*>::iterator us;
    int count = 0;

    for (it = m_docModels.begin(); it != m_docModels.end();) {
        if (it.value()->ref.load() <= 1) {
            us = m_undoStatcks.find(it.value());
            if (us != m_undoStatcks.end()) {
                delete us.value();
                m_undoStatcks.erase(us);
            }

            m_loadingDocs.remove(it.value()->document());
            delete it.value();
            it = m_docModels.erase(it);
            ++count;
        }
        else {
            ++it;
        }
    }

    return count;
}

GtDocManager::GtDocManager(QThread *thread, QObject *parent)
    : QObject(parent)
    , d_ptr(new GtDocManagerPrivate(this))
{
    d_ptr->m_docThread = thread;
}

GtDocManager::~GtDocManager()
{
}

int GtDocManager::registerLoaders(const QString &loaderDir)
{
    Q_D(GtDocManager);
    return d->m_docLoader->registerLoaders(loaderDir);
}

GtDocMeta *GtDocManager::loadDocMeta(const QString &fileId)
{
    Q_D(GtDocManager);

    QHash<QString, GtDocMeta*>::iterator it;

    it = d->m_docMetas.find(fileId);
    if (it != d->m_docMetas.end())
        return it.value();

    d->clearDocMetas();

    QSqlQuery query(d->m_docDatabase);
    query.prepare("SELECT * FROM document WHERE uuid=':fildid'");
    if (query.exec()) {
        if (query.next()) {
            qDebug() << query.value(2).toString()
                     << query.value(3).toString()
                     << query.value(4).toString();
        }
    }

    GtDocMeta *meta = new GtDocMeta(fileId);
    meta->ref.ref();
    d->m_docMetas.insert(fileId, meta);

    return meta;
}

GtDocModel *GtDocManager::loadDocument(const QString &fileId)
{
    return 0;
}

GtDocModel *GtDocManager::loadLocalDocument(const QString &fileName)
{
    Q_D(GtDocManager);

    QHash<QString, GtDocModel*>::iterator it;

    it = d->m_docModels.find(fileName);
    if (it != d->m_docModels.end())
        return it.value();

    if (d->clearDocuments() > 0)
        d->clearDocMetas();

    GtDocument *document;
    GtDocModel *model = new GtDocModel();

    document = d->m_docLoader->loadDocument(fileName, d->m_docThread, model);
    if (document) {
        model->setDocument(document);
        model->setMinScale(0.1);
        model->setMaxScale(4.0);
        model->setMouseMode(GtDocModel::SelectText);

        GtDocNotes *notes = new GtDocNotes(model);
        model->setNotes(notes);

        GtBookmarks *bookmarks = new GtBookmarks(model);
        model->setBookmarks(bookmarks);

        model->ref.ref();
        d->m_docModels.insert(fileName, model);
        d->m_loadingDocs.insert(document, model);

        if (!document->isLoaded()) {
            connect(document,
                    SIGNAL(loaded(GtDocument*)),
                    this,
                    SLOT(documentLoaded(GtDocument*)));
        }

        if (document->isLoaded())
            documentLoaded(document);
    }

    return model;
}

QUndoStack *GtDocManager::undoStack(GtDocModel *docModel)
{
    Q_D(GtDocManager);

    QHash<GtDocModel*, QUndoStack*>::iterator it;

    it = d->m_undoStatcks.find(docModel);
    if (it != d->m_undoStatcks.end())
        return it.value();

    QUndoStack *undoStack = new QUndoStack(this);
    d->m_undoStatcks.insert(docModel, undoStack);
    return undoStack;
}

void GtDocManager::documentLoaded(GtDocument *document)
{
    Q_D(GtDocManager);

    QHash<GtDocument*, GtDocModel*>::iterator it;

    it = d->m_loadingDocs.find(document);
    if (it == d->m_loadingDocs.end()) {
        qWarning() << "invalid loading document:"
                   << document->title();
        return;
    }

    GtDocModel *docModel = it.value();
    d->m_loadingDocs.erase(it);

    if (document->isLoaded()) {
        GtBookmarks *bookmarks = docModel->bookmarks();
        document->loadOutline(bookmarks->root());
    }
}

GT_END_NAMESPACE
