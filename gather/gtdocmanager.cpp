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
    int cleanDocMetas();
    int cleanDocuments();
    int cleanBookmarks();
    int cleanNotes();

protected:
    GtDocManager *q_ptr;
    GtDocLoader *m_docLoader;
    QThread *m_docThread;
    QHash<GtDocument*, GtDocModel*> m_loadingDocs;
    QHash<GtDocModel*, QUndoStack*> m_undoStatcks;
    QHash<QString, GtDocMeta*> m_docMetas;
    QHash<QString, GtDocModel*> m_docModels;
    QHash<QString, GtBookmarks*> m_bookmarks;
    QHash<QString, GtDocNotes*> m_notes;
    QSqlDatabase m_docDatabase;
};

GtDocManagerPrivate::GtDocManagerPrivate(GtDocManager *q)
    : q_ptr(q)
    , m_docThread(0)
{
    m_docLoader = new GtDocLoader(q);

    // find sqlite driver
    m_docDatabase = QSqlDatabase::addDatabase("QSQLITE");

    QString dbpath = GtApplication::dataFilePath("document.db");
    m_docDatabase.setDatabaseName(dbpath);

    if (m_docDatabase.open()) {
        QString sql = "CREATE TABLE IF NOT EXISTS document "
                      "(id INTEGER PRIMARY KEY, "
                      "uuid VARCHAR(64), "
                      "bookmarks VARCHAR(64), "
                      "notes VARCHAR(64), "
                      "path VARCHAR(256))";

        QSqlQuery query(m_docDatabase);
        if (!query.exec(sql)) {
            qWarning() << "create document table error:"
                       << query.lastError();
        }

        sql = "CREATE TABLE IF NOT EXISTS bookmarks "
              "(id integer primary key, "
              "uuid VARCHAR(64), "
              "data BLOB)";

        if (!query.exec(sql)) {
            qWarning() << "create bookmarks table error:"
                       << query.lastError();
        }

        sql = "CREATE TABLE IF NOT EXISTS notes "
              "(id integer primary key, "
              "uuid VARCHAR(64), "
              "data BLOB)";

        if (!query.exec(sql)) {
            qWarning() << "create notes table error:"
                       << query.lastError();
        }
    }
    else {
        qWarning() << "open document database error:"
                   << m_docDatabase.lastError();
    }
}

GtDocManagerPrivate::~GtDocManagerPrivate()
{
    cleanDocuments();
    cleanDocMetas();
    cleanBookmarks();
    cleanNotes();

    m_docDatabase.close();

    Q_ASSERT(m_undoStatcks.size() == 0);
    Q_ASSERT(m_loadingDocs.size() == 0);
    Q_ASSERT(m_docModels.size() == 0);
    Q_ASSERT(m_docMetas.size() == 0);
    Q_ASSERT(m_bookmarks.size() == 0);
    Q_ASSERT(m_notes.size() == 0);
}

int GtDocManagerPrivate::cleanDocMetas()
{
    // clean up any unreferenced doc metas
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

int GtDocManagerPrivate::cleanDocuments()
{
    // clean up any unreferenced documents
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

int GtDocManagerPrivate::cleanBookmarks()
{
    // clean up any unreferenced bookmarks
    QHash<QString, GtBookmarks*>::iterator it;
    int count = 0;

    for (it = m_bookmarks.begin(); it != m_bookmarks.end();) {
        if (it.value()->ref.load() <= 1) {
            delete it.value();
            it = m_bookmarks.erase(it);
            ++count;
        }
        else {
            ++it;
        }
    }

    return count;
}

int GtDocManagerPrivate::cleanNotes()
{
    // clean up any unreferenced notes
    QHash<QString, GtDocNotes*>::iterator it;
    int count = 0;

    for (it = m_notes.begin(); it != m_notes.end();) {
        if (it.value()->ref.load() <= 1) {
            delete it.value();
            it = m_notes.erase(it);
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

    d->cleanDocMetas();

    GtDocMeta *meta = 0;
    QSqlQuery query(d->m_docDatabase);
    query.prepare("SELECT * FROM document WHERE uuid=':fildid'");
    if (query.exec()) {
        if (query.next()) {
            qDebug() << query.value(2).toString()
                     << query.value(3).toString()
                     << query.value(4).toString();
        }
    }

    if (!meta) {
        meta = new GtDocMeta(fileId);
        meta->ref.ref();
        d->m_docMetas.insert(fileId, meta);
    }

    connect(meta,
            SIGNAL(changed(GtDocMeta *)),
            this,
            SLOT(docMetaChanged(GtDocMeta *)));

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

    if (d->cleanDocuments() > 0)
        d->cleanDocMetas();

    GtDocument *document;
    GtDocModel *model = new GtDocModel();

    document = d->m_docLoader->loadDocument(fileName, d->m_docThread, model);
    if (document) {
        model->setDocument(document);
        model->setMinScale(0.1);
        model->setMaxScale(4.0);
        model->setMouseMode(GtDocModel::SelectText);

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

GtBookmarks *GtDocManager::loadBookmarks(const QString &bookmarksId)
{
    return 0;
}

GtDocNotes *GtDocManager::loadNotes(const QString &notesId)
{
    return 0;
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

    GtDocModel *model = it.value();
    d->m_loadingDocs.erase(it);

    if (!document->isLoaded())
        return;

    GtDocMeta *meta = loadDocMeta(document->fileId());
    if (!meta) {
        qWarning() << "load document meta failed:"
                   << document->title();
        return;
    }

    bool metaChanged = false;

    GtBookmarks *bookmarks = 0;
    QString bookmarksId = meta->bookmarksId();
    if (bookmarksId.isEmpty()) {
        bookmarksId = GtApplication::generateUuid();
        meta->setBookmarksId(bookmarksId);

        bookmarks = new GtBookmarks(bookmarksId);
        bookmarks->ref.ref();
        document->loadOutline(bookmarks->root());
        d->m_bookmarks.insert(bookmarksId, bookmarks);

        metaChanged = true;
    }
    else {
        bookmarks = loadBookmarks(bookmarksId);
    }

    GtDocNotes *notes = 0;
    QString notesId = meta->notesId();
    if (notesId.isEmpty()) {
        notesId = GtApplication::generateUuid();
        meta->setNotesId(notesId);

        notes = new GtDocNotes(notesId);
        notes->ref.ref();
        d->m_notes.insert(notesId, notes);

        metaChanged = true;
    }
    else {
        notes = loadNotes(notesId);
    }

    model->setNotes(notes);
    model->setBookmarks(bookmarks);

    if (metaChanged)
        emit meta->changed(meta);
}

void GtDocManager::docMetaChanged(GtDocMeta *meta)
{
    qDebug() << ">>>>>>>>>>>>>>>>>>>>>";
}

GT_END_NAMESPACE
