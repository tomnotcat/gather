/*
 * Copyright (C) 2013 Tom Wong. All rights reserved.
 */
#include "pdfdocument.h"
#include "gtdocument_p.h"
#include <QtCore/QDebug>
#include <QtCore/QMutex>
#include <QtCore/QThread>

extern "C" {
#include "mupdf-internal.h"
}

GT_BEGIN_NAMESPACE

class PdfDocumentPrivate : public GtDocumentPrivate
{
    Q_DECLARE_PUBLIC(PdfDocument)

public:
    PdfDocumentPrivate();
    ~PdfDocumentPrivate();

public:
    static fz_context* context();
    static void lockContext(void *user, int lock);
    static void unlockContext(void *user, int lock);
    static int readPdfStream(fz_stream *stm, unsigned char *buf, int len);
    static void seekPdfStream(fz_stream *stm, int offset, int whence);
    static void closePdfStream(fz_context *ctx, void *state);

private:
    fz_document *document;
};

PdfDocumentPrivate::PdfDocumentPrivate()
    : document(0)
{
}

PdfDocumentPrivate::~PdfDocumentPrivate()
{
    if (document) {
        fz_close_document(document);
        document = NULL;
    }
}

fz_context* PdfDocumentPrivate::context()
{
    // Thread ID to context
    static QHash<int, fz_context*> contexts;
    static QMutex mutex;
    int threadId = reinterpret_cast<int>(QThread::currentThreadId());
    fz_context *context = 0;
    bool done = false;

    if (contexts.size() == 0) {
        mutex.lock();

        if (contexts.size() == 0) {
        }

        mutex.unlock();
    }

    mutex.lock();

    context = contexts.take(threadId);
    if (context)
        done = true;
    else if (contexts.size() > 0)
        context = *contexts.begin();

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

        context = fz_new_context(NULL, &locks, FZ_STORE_UNLIMITED);
    }

    qDebug() << "Create pdf context for thread:" << threadId;

    mutex.lock();

    if (contexts.contains(threadId)) {
        fz_free_context(context);
        context = contexts.take(threadId);
    }
    else {
        contexts.insert(threadId, context);
    }

    mutex.unlock();

    return context;
}

void PdfDocumentPrivate::lockContext(void *user, int lock)
{
    QMutex *mutexs = static_cast<QMutex*>(user);
    mutexs[lock].lock();
}

void PdfDocumentPrivate::unlockContext(void *user, int lock)
{
    QMutex *mutexs = static_cast<QMutex*>(user);
    mutexs[lock].unlock();
}

int PdfDocumentPrivate::readPdfStream(fz_stream *stm, unsigned char *buf, int len)
{
    return 0;
}

void PdfDocumentPrivate::seekPdfStream(fz_stream *stm, int offset, int whence)
{
}

void PdfDocumentPrivate::closePdfStream(fz_context *ctx, void *state)
{
}

PdfDocument::PdfDocument(QObject *parent)
    : GtDocument(*new PdfDocumentPrivate(), parent)
{
}

PdfDocument::~PdfDocument()
{
}

bool PdfDocument::loadDocument(QIODevice *device)
{
    Q_D(PdfDocument);

    fz_context *context;
    fz_stream *stream;

    context = PdfDocumentPrivate::context();
    stream = fz_new_stream(context, device,
                           PdfDocumentPrivate::readPdfStream,
                           PdfDocumentPrivate::closePdfStream);
    stream->seek = PdfDocumentPrivate::seekPdfStream;
    d->document = fz_open_document_with_stream(context, "pdf", stream);
    fz_close (stream);
    qDebug() << "load pdf";
    return true;
}

int PdfDocument::countPages()
{
    return 0;
}

GtDocPage* PdfDocument::loadPage(int page)
{
    Q_UNUSED(page);
    return 0;
}

GtDocument* gather_load_document(QIODevice *device)
{
    QScopedPointer<PdfDocument> doc(new PdfDocument());

    if (!doc->loadDocument(device))
        return 0;

    return doc.take();
}

GT_END_NAMESPACE
