/*
 * Copyright (C) 2013 Tom Wong. All rights reserved.
 */
#include "gtdocument_p.h"
#include "gtabstractdocument.h"
#include "gtdocoutline.h"
#include "gtdocpage_p.h"
#include <QtCore/QCryptographicHash>
#include <QtCore/QDebug>

GT_BEGIN_NAMESPACE

GtDocumentPrivate::GtDocumentPrivate(GtAbstractDocument *ad)
    : device(0)
    , pages(0)
    , outline(0)
    , pageCount(0)
    , maxWidth(0)
    , maxHeight(0)
    , minWidth(0)
    , minHeight(0)
    , uniform(false)
    , loaded(false)
    , destroyed(false)
    , abstractDoc(ad)
{
}

GtDocumentPrivate::~GtDocumentPrivate()
{
    for (int i = 0; i < pageCount; ++i)
        delete pages[i];

    delete[] pages;
    delete outline;

    destroyed = true;
}

void GtDocumentPrivate::setDevice(QIODevice *device)
{
    Q_ASSERT(0 == this->device);

    this->device = device;

    q_ptr->connect(device,
                   SIGNAL(destroyed(QObject*)),
                   q_ptr,
                   SLOT(deviceDestroyed(QObject*)));
}

GtAbstractPage* GtDocumentPrivate::lockPage(int index)
{
    Q_ASSERT(index >= 0 && index < pageCount);

    _mutex.lock();

    if (0 == pages[index]->d_ptr->abstractPage) {
        pages[index]->d_ptr->abstractPage = abstractDoc->loadPage(index);
        cachedPage.append(index);

        const int pageCacheSize = 16;
        if (cachedPage.size() > pageCacheSize) {
            GtDocPage *temp = pages[cachedPage.front()];

            if (temp->d_ptr->abstractPage) {
                delete temp->d_ptr->abstractPage;
                temp->d_ptr->abstractPage = 0;
            }

            cachedPage.pop_front();
        }
    }

    return pages[index]->d_ptr->abstractPage;
}

void GtDocumentPrivate::unlockPage(int index)
{
    Q_ASSERT(index >= 0 && index < pageCount);
    _mutex.unlock();
}

void GtDocumentPrivate::cacheText(int index, const GtDocTextPointer &text)
{
    Q_ASSERT(index >= 0 && index < pageCount);

    QMutexLocker lock(&_mutex);
    if (!pages[index]->d_ptr->text) {
        pages[index]->d_ptr->text = text;
        cachedText.append(index);

        const int textCacheSize = 16;
        if (cachedText.size() > textCacheSize) {
            int removeCount = cachedText.size() - textCacheSize;

            QList<int>::iterator it = cachedText.begin();
            while (removeCount > 0 && it != cachedText.end()) {
                if (*it != index && pages[*it]->d_ptr->text->ref.load() < 2) {
                    pages[*it]->d_ptr->text = 0;
                    it = cachedText.erase(it);
                    --removeCount;
                }
                else {
                    ++it;
                }
            }

            if (removeCount > 0)
                qWarning() << "page text cache surpass:" << removeCount;
        }
    }
    else {
        qWarning() << "page text already cached:" << index;
    }
}

void GtDocumentPrivate::loadOutline(GtAbstractOutline *outline,
                                    GtDocOutline *parent,
                                    GtDocOutline **node, void *it)
{
    if (0 == node)
        node = &parent->_child;

    int row = 0;
    while (it) {
        *node = new GtDocOutline(outline->title(it), outline->page(it), row++);
        (*node)->_parent = parent;

        void *child = outline->childNode(it);
        if (child) {
            loadOutline(outline, *node, &(*node)->_child, child);
            outline->freeNode(child);
        }

        it = outline->nextNode(it);
        node = &(*node)->_next;
    }

    if (row > 0)
        parent->_childCount = row;
}

GtDocument::GtDocument(GtAbstractDocument *ad,
                       QObject *parent)
    : QObject(parent)
    , d_ptr(new GtDocumentPrivate(ad))
{
    d_ptr->q_ptr = this;
}

GtDocument::GtDocument(const GtDocument &o)
    : QObject(0)
    , QSharedData(o)
{
    // tricks for QSharedDataPointer, but should not happen
    Q_ASSERT(0);
}

GtDocument::~GtDocument()
{
}

bool GtDocument::isLoaded() const
{
    Q_D(const GtDocument);
    return d->loaded;
}

bool GtDocument::isPageSizeUniform() const
{
    Q_D(const GtDocument);
    Q_ASSERT(d->loaded);
    return d->uniform;
}

QSize GtDocument::maxPageSize(double scale, int rotation) const
{
    Q_D(const GtDocument);

    Q_ASSERT(d->loaded);

    double width = d->maxWidth * scale;
    double height = d->maxHeight * scale;

    if (rotation == 0 || rotation == 180)
        return QSize(width + 0.5, height + 0.5);

    Q_ASSERT(rotation == 90 || rotation == 270);
    return QSize(height + 0.5, width + 0.5);
}

QSize GtDocument::minPageSize(double scale, int rotation) const
{
    Q_D(const GtDocument);

    Q_ASSERT(d->loaded);

    double width = d->minWidth * scale;
    double height = d->minHeight * scale;

    if (rotation == 0 || rotation == 180)
        return QSize(width + 0.5, height + 0.5);

    Q_ASSERT(rotation == 90 || rotation == 270);
    return QSize(height + 0.5, width + 0.5);
}

int GtDocument::pageCount() const
{
    Q_D(const GtDocument);
    return d->pageCount;
}

GtDocPage* GtDocument::page(int index) const
{
    Q_D(const GtDocument);

    if (index < 0 || index >= d->pageCount)
        return 0;

    return d->pages[index];
}

GtDocOutline* GtDocument::outline() const
{
    Q_D(const GtDocument);
    return d->outline;
}

QString GtDocument::makeFileId(QIODevice *device)
{
    if (!device->isOpen() || !device->isReadable())
        return QString();

    QCryptographicHash hash(QCryptographicHash::Sha1);
    char buffer[1024];
    int length;

    while ((length = device->read(buffer, sizeof(buffer))) > 0)
        hash.addData(buffer, length);

    return QString(hash.result().toHex());
}

void GtDocument::deviceDestroyed(QObject*)
{
    Q_D(GtDocument);

    // The device should not release before document destroy
    Q_ASSERT(d->destroyed);
}

void GtDocument::slotLoadDocument()
{
    Q_D(GtDocument);

    Q_ASSERT(!d->loaded && d->device);

    if (!d->abstractDoc->load(d->device)) {
        emit loaded(this);
        return;
    }

    d->pageCount = d->abstractDoc->countPages();
    if (d->pageCount > 0) {
        double pageWidth, pageHeight;
        double uniformWidth, uniformHeight;

        d->pages = new GtDocPage*[d->pageCount];
        d->uniform = true;
        for (int i = 0; i < d->pageCount; ++i) {
            QScopedPointer<GtAbstractPage> page(d->abstractDoc->loadPage(i));
            if (0 == page) {
                qWarning() << "load page failed:" << i;
                continue;
            }

            d->pages[i] = new GtDocPage();
            page->size(&pageWidth, &pageHeight);
            d->pages[i]->d_ptr->initialize(this, i, pageWidth, pageHeight);
            if (i == 0) {
                uniformWidth = pageWidth;
                uniformHeight = pageHeight;
                d->maxWidth = uniformWidth;
                d->maxHeight = uniformHeight;
                d->minWidth = uniformWidth;
                d->minHeight = uniformHeight;
            }
            else if (d->uniform && (uniformWidth != pageWidth ||
                                    uniformHeight != pageHeight))
            {
                d->uniform = false;
            }

            if (!d->uniform) {
                if (pageWidth > d->maxWidth)
                    d->maxWidth = pageWidth;

                if (pageWidth < d->minWidth)
                    d->minWidth = pageWidth;

                if (pageHeight > d->maxHeight)
                    d->maxHeight = pageHeight;

                if (pageHeight < d->minHeight)
                    d->minHeight = pageHeight;
            }
        }
    }

    // load outline
    GtAbstractOutline *outline = d->abstractDoc->loadOutline();
    d->outline = new GtDocOutline();

    void *it = outline->firstNode();
    d->loadOutline(outline, d->outline, 0, it);
    outline->freeNode(it);

    delete outline;

    // make file ID
    d->device->reset();
    d->fileId = makeFileId(d->device);

    d->loaded = true;
    emit loaded(this);
}

GT_END_NAMESPACE
