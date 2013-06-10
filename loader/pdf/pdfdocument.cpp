/*
 * Copyright (C) 2013 Tom Wong. All rights reserved.
 */
#include "pdfdocument.h"
#include "pdfpage.h"
#include <QtCore/QDebug>
#include <QtCore/QMutex>
#include <QtCore/QThread>

GT_BEGIN_NAMESPACE

PdfDocument::PdfDocument()
    : document(0)
{
}

PdfDocument::~PdfDocument()
{
    if (document) {
        fz_close_document(document);
        document = NULL;
    }
}

bool PdfDocument::load(QIODevice *device)
{
    fz_context *context;
    fz_stream *stream;

    context = PdfDocument::context();
    stream = fz_new_stream(context, device,
                           PdfDocument::readPdfStream,
                           PdfDocument::closePdfStream);
    stream->seek = PdfDocument::seekPdfStream;

    fz_try(context) {
        document = fz_open_document_with_stream(context, "pdf", stream);
    }
    fz_catch(context) {
        if (document) {
            fz_close_document(document);
            document = 0;
        }
    }

    fz_close (stream);

    return (document != 0);
}

int PdfDocument::countPages()
{
    return fz_count_pages(document);
}

GtAbstractPage* PdfDocument::loadPage(int index)
{
    fz_page *page = fz_load_page(document, index);
    if (0 == page)
        return 0;

    return new PdfPage(document, page);
}

fz_context* PdfDocument::context()
{
    // Thread ID to context
    static QHash<Qt::HANDLE, fz_context*> contexts;
    static QMutex mutex;
    Qt::HANDLE threadId = QThread::currentThreadId();
    fz_context *context = 0;
    bool done = false;

    mutex.lock();

    if (contexts.contains(threadId)) {
        context = contexts[threadId];
        done = true;
    }
    else if (contexts.size() > 0) {
        context = *contexts.begin();
    }

    mutex.unlock();

    if (done)
        return context;

    if (context) {
        context = fz_clone_context(context);
    }
    else {
        static fz_locks_context locks;
        static QMutex *mutexs[FZ_LOCK_MAX];

        mutex.lock();
        if (0 == mutexs[0]) {
            for (int i = 0; i < FZ_LOCK_MAX; ++i)
                mutexs[i] = new QMutex();
        }

        locks.user = mutexs;
        locks.lock = lockContext;
        locks.unlock = unlockContext;
        mutex.unlock();

        context = fz_new_context(NULL, &locks, FZ_STORE_DEFAULT);
    }

    qDebug() << "Create fz_context for thread:" << threadId;

    mutex.lock();

    if (contexts.contains(threadId)) {
        fz_free_context(context);
        context = contexts[threadId];
    }
    else {
        contexts.insert(threadId, context);
    }

    mutex.unlock();

    return context;
}

void PdfDocument::lockContext(void *user, int lock)
{
    QMutex **mutexs = static_cast<QMutex**>(user);
    mutexs[lock]->lock();
}

void PdfDocument::unlockContext(void *user, int lock)
{
    QMutex **mutexs = static_cast<QMutex**>(user);
    mutexs[lock]->unlock();
}

int PdfDocument::readPdfStream(fz_stream *stm, unsigned char *buf, int len)
{
    QIODevice *device = static_cast<QIODevice*>(stm->state);
    return device->read((char*)buf, len);
}

void PdfDocument::seekPdfStream(fz_stream *stm, int offset, int whence)
{
    QIODevice *device = static_cast<QIODevice*>(stm->state);
    qint64 pos = 0;

    switch (whence) {
    case SEEK_SET:
        break;

    case SEEK_CUR:
        pos = device->pos();
        break;

    case SEEK_END:
        pos = device->size();
        break;

    default:
        Q_ASSERT(0);
    }

    pos += offset;

    if (device->seek(pos)) {
        stm->pos = pos;
        stm->rp = stm->bp;
        stm->wp = stm->bp;
    }
    else {
        qWarning() << "pdf seek error:" << whence << offset;
    }
}

void PdfDocument::closePdfStream(fz_context*, void *state)
{
    QIODevice *device = static_cast<QIODevice*>(state);
    device->close();
}

GT_DEFINE_DOCUMENT_LOADER(PdfDocument())

GT_END_NAMESPACE
