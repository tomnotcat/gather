/*
 * Copyright (C) 2013 Tom Wong. All rights reserved.
 */
#include "gtdocmanager.h"
#include "gtbookmarks.h"
#include "gtdocloader.h"
#include "gtdocmessage.pb.h"
#include "gtdocmeta.h"
#include "gtdocmodel.h"
#include "gtdocnotes.h"
#include "gtdocument.h"
#include "gtserialize.h"
#include "gtuserclient.h"
#include <QtCore/QDebug>
#include <QtCore/QTimer>
#include <QtCore/QUuid>
#include <QtSql/QSqlError>
#include <QtSql/QSqlQuery>
#include <QtWidgets/QUndoStack>

GT_BEGIN_NAMESPACE

class GtDocManagerPrivate
{
    Q_DECLARE_PUBLIC(GtDocManager)

public:
    GtDocManagerPrivate(GtDocManager *q, QThread *t);
    ~GtDocManagerPrivate();

public:
    void openDatabase(const QString &dbpath);
    void updateDatabase();
    void bookmarksChanged(GtBookmarks *bookmarks);
    void notesChanged(GtDocNotes *notes);
    bool readDocMetaFromDB(GtDocMeta *meta);
    bool readBookmarksFromDB(GtBookmarks *bookmarks);
    bool readDocNotesFromDB(GtDocNotes *notes);
    bool writeDocMetaToDB(const GtDocMeta *meta);
    bool writeBookmarksToDB(const GtBookmarks *bookmarks);
    bool writeDocNotesToDB(const GtDocNotes *notes);
    int cleanDocMetas();
    int cleanBookmarks();
    int cleanDocNotes();
    int cleanDocuments();

public:
    template<typename T0, typename T1>
    bool readFromDatabase(const QString &table, T0 &dest);

    template<typename T0, typename T1>
    bool writeToDatabase(const QString &table, const T0 &src);

protected:
    GtDocManager *q_ptr;
    GtDocLoader *m_docLoader;

    QHash<QString, QString> m_path2id;
    QHash<QString, GtDocModel*> m_docModels;
    QHash<GtDocModel*, QUndoStack*> m_undoStatcks;

    QHash<QString, GtDocMeta*> m_docMetas;
    QHash<QString, GtBookmarks*> m_bookmarks;
    QHash<QString, QString> m_tempBookmarks;
    QHash<QString, GtDocNotes*> m_docNotes;
    QHash<QString, QString> m_tempDocNotes;

    QSet<GtDocMeta*> m_changedMetas;
    QSet<GtBookmarks*> m_changedBookmarks;
    QSet<GtDocNotes*> m_changedNotes;
    int m_changedCount;

    QTimer m_updateDatabaseTimer;
    QSqlDatabase m_docDatabase;
    bool m_hasDatabase;
};

GtDocManagerPrivate::GtDocManagerPrivate(GtDocManager *q, QThread *t)
    : q_ptr(q)
    , m_changedCount(0)
    , m_hasDatabase(false)
{
    m_docLoader = new GtDocLoader(t, q);
}

GtDocManagerPrivate::~GtDocManagerPrivate()
{
    cleanDocuments();
    cleanDocMetas();
    cleanBookmarks();
    cleanDocNotes();

    if (m_hasDatabase)
        m_docDatabase.close();

    Q_ASSERT(m_undoStatcks.size() == 0);
    Q_ASSERT(m_docModels.size() == 0);
    Q_ASSERT(m_docMetas.size() == 0);
    Q_ASSERT(m_bookmarks.size() == 0);
    Q_ASSERT(m_tempBookmarks.size() == 0);
    Q_ASSERT(m_docNotes.size() == 0);
    Q_ASSERT(m_tempDocNotes.size() == 0);
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
        sql = "CREATE TABLE IF NOT EXISTS docmeta "
              "(id INTEGER PRIMARY KEY AUTOINCREMENT, "
              "uuid VARCHAR(64), "
              "data BLOB)";

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

        // document notes table
        sql = "CREATE TABLE IF NOT EXISTS docnotes "
              "(id INTEGER PRIMARY KEY AUTOINCREMENT, "
              "uuid VARCHAR(64), "
              "data BLOB)";

        if (!query.exec(sql)) {
            qWarning() << "create doc notes table error:"
                       << query.lastError();
        }

        m_hasDatabase = true;
    }
    else {
        qWarning() << "open document database error:"
                   << m_docDatabase.lastError();
    }
}

void GtDocManagerPrivate::updateDatabase()
{
    if (!m_hasDatabase)
        return;

    if (m_changedCount++ < 10) {
        m_updateDatabaseTimer.start(10000);
    }
}

void GtDocManagerPrivate::bookmarksChanged(GtBookmarks *bookmarks)
{
    QHash<QString, QString>::iterator it0;

    it0 = m_tempBookmarks.find(bookmarks->id());
    if (it0 != m_tempBookmarks.end()) {
        QHash<QString, GtDocMeta*>::iterator it1;

        it1 = m_docMetas.find(it0.value());
        if (it1 != m_docMetas.end())
            m_changedMetas.insert(it1.value());

        m_tempBookmarks.erase(it0);
    }

    m_changedBookmarks.insert(bookmarks);
    updateDatabase();
}

void GtDocManagerPrivate::notesChanged(GtDocNotes *notes)
{
    QHash<QString, QString>::iterator it0;

    it0 = m_tempDocNotes.find(notes->id());
    if (it0 != m_tempDocNotes.end()) {
        QHash<QString, GtDocMeta*>::iterator it1;

        it1 = m_docMetas.find(it0.value());
        if (it1 != m_docMetas.end())
            m_changedMetas.insert(it1.value());

        m_tempDocNotes.erase(it0);
    }

    m_changedNotes.insert(notes);
    updateDatabase();
}

bool GtDocManagerPrivate::readDocMetaFromDB(GtDocMeta *meta)
{
    return readFromDatabase<GtDocMeta, GtDocMetaMsg>("docmeta", *meta);
}

bool GtDocManagerPrivate::readBookmarksFromDB(GtBookmarks *bookmarks)
{
    return readFromDatabase<GtBookmarks, GtBookmarksMsg>("bookmarks", *bookmarks);
}

bool GtDocManagerPrivate::readDocNotesFromDB(GtDocNotes *notes)
{
    return readFromDatabase<GtDocNotes, GtDocNotesMsg>("docnotes", *notes);
}

bool GtDocManagerPrivate::writeDocMetaToDB(const GtDocMeta *meta)
{
    GtDocMeta temp(meta->id());

    if (!m_tempBookmarks.contains(meta->bookmarksId()))
        temp.setBookmarksId(meta->bookmarksId());

    if (!m_tempDocNotes.contains(meta->notesId()))
        temp.setNotesId(meta->notesId());

    return writeToDatabase<GtDocMeta, GtDocMetaMsg>("docmeta", temp);
}

bool GtDocManagerPrivate::writeBookmarksToDB(const GtBookmarks *bookmarks)
{
    return writeToDatabase<GtBookmarks, GtBookmarksMsg>("bookmarks", *bookmarks);
}

bool GtDocManagerPrivate::writeDocNotesToDB(const GtDocNotes *notes)
{
    return writeToDatabase<GtDocNotes, GtDocNotesMsg>("docnotes", *notes);
}

int GtDocManagerPrivate::cleanDocMetas()
{
    // clean up any unreferenced doc metas
    QHash<QString, GtDocMeta*>::iterator it;
    int count = 0;

    for (it = m_docMetas.begin(); it != m_docMetas.end();) {
        if (it.value()->ref.load() <= 1) {
            if (m_changedMetas.contains(it.value())) {
                writeDocMetaToDB(it.value());
                m_changedMetas.remove(it.value());
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

int GtDocManagerPrivate::cleanBookmarks()
{
    // clean up any unreferenced bookmarks
    QHash<QString, GtBookmarks*>::iterator it;
    int count = 0;

    for (it = m_bookmarks.begin(); it != m_bookmarks.end();) {
        if (it.value()->ref.load() <= 1) {
            if (m_changedBookmarks.contains(it.value())) {
                writeBookmarksToDB(it.value());
                m_changedBookmarks.remove(it.value());
            }

            m_tempBookmarks.remove(it.key());
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

int GtDocManagerPrivate::cleanDocNotes()
{
    // clean up any unreferenced notes
    QHash<QString, GtDocNotes*>::iterator it;
    int count = 0;

    for (it = m_docNotes.begin(); it != m_docNotes.end();) {
        if (it.value()->ref.load() <= 1) {
            if (m_changedNotes.contains(it.value())) {
                writeDocNotesToDB(it.value());
                m_changedNotes.remove(it.value());
            }

            m_tempDocNotes.remove(it.key());
            delete it.value();
            it = m_docNotes.erase(it);
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

template<typename T0, typename T1>
bool GtDocManagerPrivate::readFromDatabase(const QString &table, T0 &dest)
{
    if (!m_hasDatabase)
        return false;

    QSqlQuery query(m_docDatabase);
    query.prepare("SELECT data FROM " + table + " WHERE uuid=:uuid");
    query.bindValue(":uuid", dest.id());

    if (!query.exec()) {
        qWarning() << "select table" << table << "error:"
                   << query.lastError();
        return false;
    }

    if (!query.first())
        return false;

    QByteArray data = query.value(0).toByteArray();
    if (!GtSerialize::deserialize<T0, T1>(dest, data)) {
        qWarning() << "deserialize" << table << "error";
        return false;
    }

    return true;
}

template<typename T0, typename T1>
bool GtDocManagerPrivate::writeToDatabase(const QString &table, const T0 &src)
{
    if (!m_hasDatabase)
        return false;

    QSqlQuery query(m_docDatabase);
    query.prepare("SELECT uuid FROM " + table + " WHERE uuid=:uuid");
    query.bindValue(":uuid", src.id());

    if (!query.exec()) {
        qWarning() << "select table" << table << "error:"
                   << query.lastError();
        return false;
    }

    QByteArray data;
    if (!GtSerialize::serialize<T0, T1>(src, data)) {
        qWarning() << "serialize" << table << "error";
        return false;
    }

    if (query.first()) {
        query.prepare("UPDATE " + table + " SET data=:data WHERE uuid=:uuid");
        query.bindValue(":uuid", src.id());
        query.bindValue(":data", data);

        if (!query.exec()) {
            qWarning() << "update table" << table << "error:"
                       << query.lastError();
            return false;
        }
    }
    else {
        query.prepare("INSERT INTO " + table + " (uuid, data) "
                      "VALUES(:uuid, :data)");
        query.bindValue(":uuid", src.id());
        query.bindValue(":data", data);

        if (!query.exec()) {
            qWarning() << "insert table" << table << "error:"
                       << query.lastError();
            return false;
        }
    }

    return true;
}

GtDocManager::GtDocManager(const QString &dbpath,
                           QThread *thread,
                           QObject *parent)
    : QObject(parent)
    , d_ptr(new GtDocManagerPrivate(this, thread))
{
    if (!dbpath.isEmpty()) {
        d_ptr->openDatabase(dbpath);

        d_ptr->m_updateDatabaseTimer.setSingleShot(true);
        connect(&d_ptr->m_updateDatabaseTimer, SIGNAL(timeout()),
                this, SLOT(updateDatabase()));
    }
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

int GtDocManager::cleanUnreferencedObjects()
{
    Q_D(GtDocManager);

    return d->cleanDocuments() +
            d->cleanDocMetas() +
            d->cleanBookmarks() +
            d->cleanDocNotes();
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
    if (d->m_hasDatabase)
        d->readDocMetaFromDB(meta);

    meta->ref.ref();
    d->m_docMetas.insert(fileId, meta);

    return meta;
}

GtBookmarks *GtDocManager::loadBookmarks(const QString &bookmarksId)
{
    Q_D(GtDocManager);

    QHash<QString, GtBookmarks*>::iterator it;

    it = d->m_bookmarks.find(bookmarksId);
    if (it != d->m_bookmarks.end())
        return it.value();

    d->cleanBookmarks();

    GtBookmarks *bookmarks = new GtBookmarks(bookmarksId);
    if (d->m_hasDatabase)
        d->readBookmarksFromDB(bookmarks);

    connect(bookmarks, SIGNAL(added(GtBookmark*)),
            this, SLOT(bookmarkAdded(GtBookmark*)));

    connect(bookmarks, SIGNAL(removed(GtBookmark*)),
            this, SLOT(bookmarkRemoved(GtBookmark*)));

    connect(bookmarks, SIGNAL(updated(GtBookmark*, int)),
            this, SLOT(bookmarkUpdated(GtBookmark*, int)));

    bookmarks->ref.ref();
    d->m_bookmarks.insert(bookmarksId, bookmarks);

    return bookmarks;
}

GtDocNotes *GtDocManager::loadDocNotes(const QString &notesId)
{
    Q_D(GtDocManager);

    QHash<QString, GtDocNotes*>::iterator it;

    it = d->m_docNotes.find(notesId);
    if (it != d->m_docNotes.end())
        return it.value();

    d->cleanDocNotes();

    GtDocNotes *notes = new GtDocNotes(notesId);
    if (d->m_hasDatabase)
        d->readDocNotesFromDB(notes);

    connect(notes, SIGNAL(added(GtDocNote*)),
            this, SLOT(noteAdded(GtDocNote*)));

    connect(notes, SIGNAL(removed(GtDocNote*)),
            this, SLOT(noteRemoved(GtDocNote*)));

    notes->ref.ref();
    d->m_docNotes.insert(notesId, notes);

    return notes;
}

GtDocModel *GtDocManager::loadDocument(const QString &fileId)
{
    Q_ASSERT(0);
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

    d->cleanDocuments();

    GtDocument *document = d->m_docLoader->loadDocument(fileName);
    if (!document) {
        return 0;
    }

    GtDocModel *model = new GtDocModel();
    document->setParent(model);
    model->setDocument(document);
    model->setMinScale(0.1);
    model->setMaxScale(4.0);
    model->setMouseMode(GtDocModel::SelectText);

    QString fileId(document->fileId());
    d->m_path2id.insert(fileName, fileId);
    model->ref.ref();
    d->m_docModels.insert(fileId, model);

    // document meta data
    GtDocMeta *meta = loadDocMeta(fileId);
    model->setMeta(meta);

    // bookmarks
    QString bookmarksId(meta->bookmarksId());
    bool loadOutline = false;

    if (bookmarksId.isEmpty()) {
        bookmarksId = QUuid::createUuid().toString();
        meta->setBookmarksId(bookmarksId);
        loadOutline = true;
        d->m_tempBookmarks.insert(bookmarksId, fileId);
    }

    GtBookmarks *bookmarks = loadBookmarks(bookmarksId);
    model->setBookmarks(bookmarks);

    if (loadOutline) {
        if (!document->isLoaded()) {
            connect(document, SIGNAL(loaded()),
                    model, SLOT(loadOutline()));
        }

        if (document->isLoaded())
            model->loadOutline();
    }

    // document notes
    QString notesId(meta->notesId());

    if (notesId.isEmpty()) {
        notesId = QUuid::createUuid().toString();
        meta->setNotesId(notesId);
        d->m_tempDocNotes.insert(notesId, fileId);
    }

    GtDocNotes *notes = loadDocNotes(notesId);
    model->setNotes(notes);

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
    undoStack->setUndoLimit(256);
    d->m_undoStatcks.insert(docModel, undoStack);
    return undoStack;
}

void GtDocManager::bookmarkAdded(GtBookmark *bookmark)
{
    Q_D(GtDocManager);

    GtBookmarks *bookmarks = qobject_cast<GtBookmarks*>(sender());
    d->bookmarksChanged(bookmarks);
}

void GtDocManager::bookmarkRemoved(GtBookmark *bookmark)
{
    Q_D(GtDocManager);

    GtBookmarks *bookmarks = qobject_cast<GtBookmarks*>(sender());
    d->bookmarksChanged(bookmarks);
}

void GtDocManager::bookmarkUpdated(GtBookmark *bookmark, int flags)
{
    Q_D(GtDocManager);

    GtBookmarks *bookmarks = qobject_cast<GtBookmarks*>(sender());
    d->bookmarksChanged(bookmarks);
}

void GtDocManager::noteAdded(GtDocNote *note)
{
    Q_D(GtDocManager);

    GtDocNotes *notes = qobject_cast<GtDocNotes*>(sender());
    d->notesChanged(notes);
}

void GtDocManager::noteRemoved(GtDocNote *note)
{
    Q_D(GtDocManager);

    GtDocNotes *notes = qobject_cast<GtDocNotes*>(sender());
    d->notesChanged(notes);
}

void GtDocManager::updateDatabase()
{
    Q_D(GtDocManager);

    if (d->m_changedMetas.size() > 0) {
        QSet<GtDocMeta*>::iterator it;

        for (it = d->m_changedMetas.begin();
             it != d->m_changedMetas.end(); ++it)
        {
            d->writeDocMetaToDB(*it);
        }

        d->m_changedMetas.clear();
    }

    if (d->m_changedBookmarks.size() > 0) {
        QSet<GtBookmarks*>::iterator it;

        for (it = d->m_changedBookmarks.begin();
             it != d->m_changedBookmarks.end(); ++it)
        {
            d->writeBookmarksToDB(*it);
        }

        d->m_changedBookmarks.clear();
    }

    if (d->m_changedNotes.size() > 0) {
        QSet<GtDocNotes*>::iterator it;

        for (it = d->m_changedNotes.begin();
             it != d->m_changedNotes.end(); ++it)
        {
            d->writeDocNotesToDB(*it);
        }

        d->m_changedNotes.clear();
    }

    d->m_changedCount = 0;
}

GT_END_NAMESPACE
