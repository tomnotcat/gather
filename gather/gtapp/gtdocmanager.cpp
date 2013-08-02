/*
 * Copyright (C) 2013 Tom Wong. All rights reserved.
 */
#include "gtdocmanager.h"
#include "gtbookmarks.h"
#include "gtdocloader.h"
#include "gtdocmeta.h"
#include "gtdocmodel.h"
#include "gtdocnotes.h"
#include "gtdocument.h"
#include "gtuserclient.h"
#include <QtCore/QDebug>
#include <QtCore/QUuid>
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
    void openDatabase(const QString &dbpath);
    void updateDocMetaToDB(GtDocMeta *meta);
    int cleanDocMetas();
    int cleanDocuments();
    int cleanBookmarks();
    int cleanNotes();

protected:
    GtDocManager *q_ptr;
    GtDocLoader *m_docLoader;
    QThread *m_docThread;

    QHash<QString, QString> m_path2id;
    QHash<QString, GtDocModel*> m_docModels;
    QHash<GtDocument*, GtDocModel*> m_loadingDocs;
    QHash<GtDocModel*, QUndoStack*> m_undoStatcks;

    QHash<QString, GtDocMeta*> m_docMetas;
    QSet<GtDocMeta*> m_changedDocMetas;
    QHash<QString, GtBookmarks*> m_bookmarks;
    QHash<QString, GtDocNotes*> m_notes;
    QHash<QString, GtDocMeta*> m_initBmNotes;

    QSqlDatabase m_docDatabase;
    bool m_hasDatabase;
};

GtDocManagerPrivate::GtDocManagerPrivate(GtDocManager *q)
    : q_ptr(q)
    , m_docThread(0)
    , m_hasDatabase(false)
{
    m_docLoader = new GtDocLoader(q);
}

GtDocManagerPrivate::~GtDocManagerPrivate()
{
    cleanDocuments();
    cleanDocMetas();
    cleanBookmarks();
    cleanNotes();

    if (m_hasDatabase)
        m_docDatabase.close();

    Q_ASSERT(m_undoStatcks.size() == 0);
    Q_ASSERT(m_loadingDocs.size() == 0);
    Q_ASSERT(m_docModels.size() == 0);
    Q_ASSERT(m_docMetas.size() == 0);
    Q_ASSERT(m_changedDocMetas.size() == 0);
    Q_ASSERT(m_bookmarks.size() == 0);
    Q_ASSERT(m_notes.size() == 0);
}

void GtDocManagerPrivate::openDatabase(const QString &dbpath)
{
    // find sqlite driver
    m_docDatabase = QSqlDatabase::addDatabase("QSQLITE");
    m_docDatabase.setDatabaseName(dbpath);

    if (m_docDatabase.open()) {
        QSqlQuery query(m_docDatabase);

        // file ID to file path table
        QString sql = "CREATE TABLE IF NOT EXISTS id2path "
                      "(id INTEGER PRIMARY KEY AUTOINCREMENT, "
                      "uuid VARCHAR(64), "
                      "path VARCHAR(256))";

        if (!query.exec(sql)) {
            qWarning() << "create id to path table error:"
                       << query.lastError();
        }

        // doc meta table
        sql = "CREATE TABLE IF NOT EXISTS meta "
              "(id INTEGER PRIMARY KEY AUTOINCREMENT, "
              "uuid VARCHAR(64), "
              "bookmarks VARCHAR(64), "
              "notes VARCHAR(64))";

        if (!query.exec(sql)) {
            qWarning() << "create doc meta table error:"
                       << query.lastError();
        }

        // bookmarks table
        sql = "CREATE TABLE IF NOT EXISTS bookmarks "
              "(id INTEGER PRIMARY KEY AUTOINCREMENT, "
              "uuid VARCHAR(64), "
              "data BLOB)";

        if (!query.exec(sql)) {
            qWarning() << "create bookmarks table error:"
                       << query.lastError();
        }

        // notes table
        sql = "CREATE TABLE IF NOT EXISTS notes "
              "(id INTEGER PRIMARY KEY AUTOINCREMENT, "
              "uuid VARCHAR(64), "
              "data BLOB)";

        if (!query.exec(sql)) {
            qWarning() << "create notes table error:"
                       << query.lastError();
        }

        m_hasDatabase = true;
    }
    else {
        qWarning() << "open document database error:"
                   << m_docDatabase.lastError();
    }
}

void GtDocManagerPrivate::updateDocMetaToDB(GtDocMeta *meta)
{
    if (!m_hasDatabase)
        return;

    QSqlQuery query(m_docDatabase);
    query.prepare("SELECT * FROM meta WHERE uuid=:uuid");
    query.bindValue(":uuid", meta->documentId());

    if (!query.exec()) {
        qWarning() << "select doc meta error:" << query.lastError();
        return;
    }

    if (query.first()) {
        query.prepare("UPDATE meta SET bookmarks=:bid, notes=:nid "
                      "WHERE uuid=:uuid");
        query.bindValue(":uuid", meta->documentId());
        query.bindValue(":bid", meta->bookmarksId());
        query.bindValue(":nid", meta->notesId());

        if (!query.exec()) {
            qWarning() << "update doc meta error:" << query.lastError();
            return;
        }
    }
    else {
        query.prepare("INSERT INTO meta (uuid, bookmarks, notes) "
                      "VALUES(:uuid, :bid, :nid)");
        query.bindValue(":uuid", meta->documentId());
        query.bindValue(":bid", meta->bookmarksId());
        query.bindValue(":nid", meta->notesId());

        if (!query.exec()) {
            qWarning() << "insert doc meta error:" << query.lastError();
            return;
        }
    }
}

int GtDocManagerPrivate::cleanDocMetas()
{
    // clean up any unreferenced doc metas
    QHash<QString, GtDocMeta*>::iterator it;
    int count = 0;

    for (it = m_docMetas.begin(); it != m_docMetas.end();) {
        if (it.value()->ref.load() <= 1) {
            if (m_changedDocMetas.contains(it.value())) {
                updateDocMetaToDB(it.value());
                m_changedDocMetas.remove(it.value());
            }

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

GtDocManager::GtDocManager(const QString &dbpath,
                           QThread *thread,
                           QObject *parent)
    : QObject(parent)
    , d_ptr(new GtDocManagerPrivate(this))
{
    d_ptr->m_docThread = thread;

    if (!dbpath.isEmpty())
        d_ptr->openDatabase(dbpath);
}

GtDocManager::~GtDocManager()
{
}

int GtDocManager::registerLoaders(const QString &loaderDir)
{
    Q_D(GtDocManager);
    return d->m_docLoader->registerLoaders(loaderDir);
}

int GtDocManager::documentCount() const
{
    Q_D(const GtDocManager);
    return d->m_docModels.size();
}

int GtDocManager::cleanDocuments()
{
    Q_D(GtDocManager);
    return d->cleanDocuments();
}

GtDocMeta *GtDocManager::loadDocMeta(const QString &fileId)
{
    Q_D(GtDocManager);

    QHash<QString, GtDocMeta*>::iterator it;

    it = d->m_docMetas.find(fileId);
    if (it != d->m_docMetas.end())
        return it.value();

    d->cleanDocMetas();

    GtDocMeta *meta = new GtDocMeta(fileId);

    if (d->m_hasDatabase) {
        QSqlQuery query(d->m_docDatabase);
        query.prepare("SELECT * FROM meta WHERE uuid=:fileid");
        query.bindValue(":fileid", fileId);

        if (query.exec() && query.first()) {
            meta->setBookmarksId(query.value(2).toString());
            meta->setNotesId(query.value(3).toString());
        }
    }

    connect(meta,
            SIGNAL(changed(GtDocMeta *)),
            this,
            SLOT(docMetaChanged(GtDocMeta *)));

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

    QHash<QString, QString>::iterator it0;

    it0 = d->m_path2id.find(fileName);
    if (it0 != d->m_path2id.end()) {
        QHash<QString, GtDocModel*>::iterator it1;

        it1 = d->m_docModels.find(it0.value());
        if (it1 != d->m_docModels.end())
            return it1.value();
    }

    if (d->cleanDocuments() > 0)
        d->cleanDocMetas();

    GtDocument *document;
    document = d->m_docLoader->loadDocument(fileName, d->m_docThread);
    if (!document) {
        return 0;
    }

    GtDocModel *model = new GtDocModel();
    document->setParent(model);
    model->setDocument(document);
    model->setMinScale(0.1);
    model->setMaxScale(4.0);
    model->setMouseMode(GtDocModel::SelectText);

    model->ref.ref();
    d->m_docModels.insert(document->fileId(), model);
    d->m_loadingDocs.insert(document, model);
    d->m_path2id.insert(fileName, document->fileId());

    if (!document->isLoaded()) {
        connect(document,
                SIGNAL(loaded(GtDocument*)),
                this,
                SLOT(documentLoaded(GtDocument*)));
    }

    if (document->isLoaded())
        documentLoaded(document);

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
        qWarning() << "invalid loaded document:" << document->title();
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

    GtBookmarks *bookmarks = 0;
    QString bookmarksId = meta->bookmarksId();
    if (bookmarksId.isEmpty()) {
        bookmarksId = QUuid::createUuid().toString();
        meta->setBookmarksId(bookmarksId);

        bookmarks = new GtBookmarks(bookmarksId);
        bookmarks->ref.ref();
        document->loadOutline(bookmarks->root());
        d->m_bookmarks.insert(bookmarksId, bookmarks);
    }
    else {
        bookmarks = loadBookmarks(bookmarksId);
    }

    GtDocNotes *notes = 0;
    QString notesId = meta->notesId();
    if (notesId.isEmpty()) {
        notesId = QUuid::createUuid().toString();
        meta->setNotesId(notesId);

        notes = new GtDocNotes(notesId);
        notes->ref.ref();
        d->m_notes.insert(notesId, notes);
    }
    else {
        notes = loadNotes(notesId);
    }

    model->setNotes(notes);
    model->setBookmarks(bookmarks);
}

void GtDocManager::docMetaChanged(const QString &)
{
    GtDocMeta *meta = qobject_cast<GtDocMeta*>(sender());
}

GT_END_NAMESPACE
