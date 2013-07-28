/*
 * Copyright (C) 2013 Tom Wong. All rights reserved.
 */
#include "gtdocmanager.h"
#include "gtbookmarks.h"
#include "gtdocloader.h"
#include "gtdocnotes.h"
#include "gtdocument.h"
#include <QtCore/QDebug>
#include <QtWidgets/QUndoStack>

GT_BEGIN_NAMESPACE

GtDocManager::GtDocManager(QThread *thread, QObject *parent)
    : QObject(parent)
    , m_docThread(thread)
{
    m_docLoader = new GtDocLoader(this);
}

GtDocManager::~GtDocManager()
{
    clearDocuments();

    Q_ASSERT(m_undoStatcks.size() == 0);
    Q_ASSERT(m_loadingDocs.size() == 0);
    Q_ASSERT(m_docModels.size() == 0);
}

int GtDocManager::registerLoaders(const QString &loaderDir)
{
    return m_docLoader->registerLoaders(loaderDir);
}

GtDocModelPointer GtDocManager::loadDocument(const QString &fileName)
{
    QHash<QString, GtDocModelPointer>::iterator it;

    it = m_docModels.find(fileName);
    if (it != m_docModels.end())
        return it.value();

    clearDocuments();

    GtDocument *document;
    GtDocModelPointer model(new GtDocModel());

    document = m_docLoader->loadDocument(fileName, m_docThread, model.data());
    if (document) {
        model->setDocument(document);
        model->setMinScale(0.1);
        model->setMaxScale(4.0);
        model->setMouseMode(GtDocModel::SelectText);

        GtDocNotes *notes = new GtDocNotes(model.data());
        model->setNotes(notes);

        GtBookmarks *bookmarks = new GtBookmarks(model.data());
        model->setBookmarks(bookmarks);

        m_docModels.insert(fileName, model);
        m_loadingDocs.insert(document, model.data());

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

QUndoStack* GtDocManager::undoStack(GtDocModel *docModel)
{
    QHash<GtDocModel*, QUndoStack*>::iterator it;

    it = m_undoStatcks.find(docModel);
    if (it != m_undoStatcks.end())
        return it.value();

    QUndoStack *undoStack = new QUndoStack(this);
    m_undoStatcks.insert(docModel, undoStack);
    return undoStack;
}

void GtDocManager::documentLoaded(GtDocument *document)
{
    QHash<GtDocument*, GtDocModel*>::iterator it;

    it = m_loadingDocs.find(document);
    if (it == m_loadingDocs.end()) {
        qWarning() << "invalid loading document:"
                   << document->title();
        return;
    }

    GtDocModel *docModel = it.value();
    m_loadingDocs.erase(it);

    if (document->isLoaded()) {
        GtBookmarks *bookmarks = docModel->bookmarks();
        document->loadOutline(bookmarks->root());
    }
}

void GtDocManager::clearDocuments()
{
    // clear up any unreferenced documents
    QHash<QString, GtDocModelPointer>::iterator it;
    QHash<GtDocModel*, QUndoStack*>::iterator us;

    for (it = m_docModels.begin(); it != m_docModels.end();) {
        if (it.value()->ref.load() <= 1) {
            us = m_undoStatcks.find(it.value().data());
            if (us != m_undoStatcks.end()) {
                delete us.value();
                m_undoStatcks.erase(us);
            }

            m_loadingDocs.remove(it.value()->document());
            it = m_docModels.erase(it);
        }
        else {
            ++it;
        }
    }
}

GT_END_NAMESPACE
