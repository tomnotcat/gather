/*
 * Copyright (C) 2013 Tom Wong. All rights reserved.
 */
#include "gtdocument_p.h"
#include "gtabstractdocument.h"
#include "gtbookmark.h"
#include "gtdocpage_p.h"
#include <QtCore/QCryptographicHash>
#include <QtCore/QDebug>

GT_BEGIN_NAMESPACE

GtDocumentPrivate::GtDocumentPrivate(GtAbstractDocument *a,
                                     GtDocument *q)
    : q_ptr(q)
    , m_device(0)
    , m_pages(0)
    , m_pageCount(0)
    , m_maxWidth(0)
    , m_maxHeight(0)
    , m_minWidth(0)
    , m_minHeight(0)
    , m_uniform(false)
    , m_loaded(false)
    , m_destroyed(false)
    , m_abstractDoc(a)
{
}

GtDocumentPrivate::~GtDocumentPrivate()
{
    for (int i = 0; i < m_pageCount; ++i)
        delete m_pages[i];

    delete[] m_pages;

    m_destroyed = true;
}

void GtDocumentPrivate::setDevice(const QString &title, QIODevice *device)
{
    Q_ASSERT(0 == m_device);

    m_title = title;
    m_device = device;

    q_ptr->connect(device,
                   SIGNAL(destroyed(QObject*)),
                   q_ptr,
                   SLOT(deviceDestroyed(QObject*)));
}

GtAbstractPage* GtDocumentPrivate::lockPage(int index)
{
    Q_ASSERT(index >= 0 && index < m_pageCount);

    m_mutex.lock();

    if (0 == m_pages[index]->d_ptr->abstractPage) {
        m_pages[index]->d_ptr->abstractPage = m_abstractDoc->loadPage(index);
        m_cachedPage.append(index);

        const int pageCacheSize = 16;
        if (m_cachedPage.size() > pageCacheSize) {
            GtDocPage *temp = m_pages[m_cachedPage.front()];

            if (temp->d_ptr->abstractPage) {
                delete temp->d_ptr->abstractPage;
                temp->d_ptr->abstractPage = 0;
            }

            m_cachedPage.pop_front();
        }
    }

    return m_pages[index]->d_ptr->abstractPage;
}

void GtDocumentPrivate::unlockPage(int index)
{
    Q_ASSERT(index >= 0 && index < m_pageCount);
    m_mutex.unlock();
}

void GtDocumentPrivate::cacheText(int index, const GtDocTextPointer &text)
{
    Q_ASSERT(index >= 0 && index < m_pageCount);

    QMutexLocker lock(&m_mutex);
    if (!m_pages[index]->d_ptr->text) {
        m_pages[index]->d_ptr->text = text;
        m_cachedText.append(index);

        const int textCacheSize = 16;
        if (m_cachedText.size() > textCacheSize) {
            int removeCount = m_cachedText.size() - textCacheSize;

            QList<int>::iterator it = m_cachedText.begin();
            while (removeCount > 0 && it != m_cachedText.end()) {
                if (*it != index &&
                    m_pages[*it]->d_ptr->text->ref.load() <= 1)
                {
                    m_pages[*it]->d_ptr->text = 0;
                    it = m_cachedText.erase(it);
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

int GtDocumentPrivate::loadOutline(GtAbstractOutline *outline,
                                   GtBookmark *parent, void *it)
{
    GtBookmark *node;
    int count = 0;

    while (it) {
        node = parent->addChild(outline->title(it), outline->dest(it));
        ++count;

        void *child = outline->childNode(it);
        if (child) {
            count += loadOutline(outline, node, child);
            outline->freeNode(child);
        }

        it = outline->nextNode(it);
    }

    return count;
}

GtDocument::GtDocument(GtAbstractDocument *a, QObject *parent)
    : QObject(parent)
    , d_ptr(new GtDocumentPrivate(a, this))
{
}

GtDocument::~GtDocument()
{
}

QString GtDocument::fileId() const
{
    Q_D(const GtDocument);
    Q_ASSERT(d->m_loaded);
    return d->m_fileId;
}

QString GtDocument::title() const
{
    Q_D(const GtDocument);
    Q_ASSERT(d->m_loaded);
    return d->m_title;
}

bool GtDocument::isLoaded() const
{
    Q_D(const GtDocument);
    return d->m_loaded;
}

bool GtDocument::isPageSizeUniform() const
{
    Q_D(const GtDocument);
    Q_ASSERT(d->m_loaded);
    return d->m_uniform;
}

QSize GtDocument::maxPageSize(double scale, int rotation) const
{
    Q_D(const GtDocument);

    Q_ASSERT(d->m_loaded);

    double width = d->m_maxWidth * scale;
    double height = d->m_maxHeight * scale;

    if (rotation == 0 || rotation == 180)
        return QSize(width + 0.5, height + 0.5);

    Q_ASSERT(rotation == 90 || rotation == 270);
    return QSize(height + 0.5, width + 0.5);
}

QSize GtDocument::minPageSize(double scale, int rotation) const
{
    Q_D(const GtDocument);

    Q_ASSERT(d->m_loaded);

    double width = d->m_minWidth * scale;
    double height = d->m_minHeight * scale;

    if (rotation == 0 || rotation == 180)
        return QSize(width + 0.5, height + 0.5);

    Q_ASSERT(rotation == 90 || rotation == 270);
    return QSize(height + 0.5, width + 0.5);
}

int GtDocument::pageCount() const
{
    Q_D(const GtDocument);
    Q_ASSERT(d->m_loaded);
    return d->m_pageCount;
}

GtDocPage* GtDocument::page(int index) const
{
    Q_D(const GtDocument);

    Q_ASSERT(d->m_loaded && index >= 0 && index < d->m_pageCount);

    return d->m_pages[index];
}

int GtDocument::loadOutline(GtBookmark *root)
{
    Q_D(GtDocument);

    Q_ASSERT(d->m_loaded);

    QMutexLocker locker(&d->m_mutex);

    GtAbstractOutline *outline = d->m_abstractDoc->loadOutline();
    if (!outline)
        return 0;

    void *it = outline->firstNode();
    int count = d->loadOutline(outline, root, it);
    outline->freeNode(it);
    delete outline;

    return count;
}

QString GtDocument::makeFileId(QIODevice *device)
{
    if (!device->isOpen() || !device->isReadable())
        return QString();

    QCryptographicHash hash(QCryptographicHash::Sha1);
    char buffer[1024];
    int length;

    qint64 pos = device->pos();
    while ((length = device->read(buffer, sizeof(buffer))) > 0)
        hash.addData(buffer, length);

    if (!device->seek(pos))
        qWarning() << "Document device not seekable";

    return QString(hash.result().toHex());
}

void GtDocument::deviceDestroyed(QObject*)
{
    Q_D(GtDocument);

    // The device should not release before document destroy
    Q_ASSERT(d->m_destroyed);
}

void GtDocument::loadDocument()
{
    Q_D(GtDocument);

    Q_ASSERT(!d->m_loaded && d->m_device);

    QMutexLocker locker(&d->m_mutex);

    d->m_fileId = makeFileId(d->m_device);
    if (!d->m_abstractDoc->load(d->m_device)) {
        locker.unlock();
        emit loaded(this);
        return;
    }

    d->m_pageCount = d->m_abstractDoc->countPages();
    if (d->m_pageCount > 0) {
        double pageWidth, pageHeight;
        double uniformWidth, uniformHeight;

        d->m_pages = new GtDocPage*[d->m_pageCount];
        d->m_uniform = true;
        for (int i = 0; i < d->m_pageCount; ++i) {
            QScopedPointer<GtAbstractPage> page(d->m_abstractDoc->loadPage(i));
            if (0 == page) {
                qWarning() << "load page failed:" << i;
                continue;
            }

            d->m_pages[i] = new GtDocPage();
            page->size(&pageWidth, &pageHeight);
            d->m_pages[i]->d_ptr->initialize(this, i, pageWidth, pageHeight);
            if (i == 0) {
                uniformWidth = pageWidth;
                uniformHeight = pageHeight;
                d->m_maxWidth = uniformWidth;
                d->m_maxHeight = uniformHeight;
                d->m_minWidth = uniformWidth;
                d->m_minHeight = uniformHeight;
            }
            else if (d->m_uniform && (uniformWidth != pageWidth ||
                                      uniformHeight != pageHeight))
            {
                d->m_uniform = false;
            }

            if (!d->m_uniform) {
                if (pageWidth > d->m_maxWidth)
                    d->m_maxWidth = pageWidth;

                if (pageWidth < d->m_minWidth)
                    d->m_minWidth = pageWidth;

                if (pageHeight > d->m_maxHeight)
                    d->m_maxHeight = pageHeight;

                if (pageHeight < d->m_minHeight)
                    d->m_minHeight = pageHeight;
            }
        }
    }

    // title
    QString title(d->m_abstractDoc->title());
    if (!title.isNull())
        d->m_title = title;

    d->m_loaded = true;

    locker.unlock();
    emit loaded(this);
}

GT_END_NAMESPACE
