/*
 * Copyright (C) 2013 Tom Wong. All rights reserved.
 */
#include "gtdocmanager.h"
#include "gtbookmarks.h"
#include "gtdocloader.h"
#include "gtdocnotes.h"
#include "gtdocument.h"
#include <QtCore/QDebug>

GT_BEGIN_NAMESPACE

GtDocManager::GtDocManager(QObject *parent)
    : QObject(parent)
    , m_docThread(0)
{
    m_docLoader = new GtDocLoader(this);
}

GtDocManager::~GtDocManager()
{
    clearDocuments();
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
        //document->loadOutline(bookmarks->root());
        model->setBookmarks(bookmarks);

        m_docModels.insert(fileName, model);
    }

    return model;
}

void GtDocManager::clearDocuments()
{
    // clear up any unreferenced documents
    QHash<QString, GtDocModelPointer>::iterator it;

    for (it = m_docModels.begin(); it != m_docModels.end();) {
        if (it.value()->ref.load() < 2) {
            it = m_docModels.erase(it);
        }
        else {
            ++it;
        }
    }
}

GT_END_NAMESPACE
