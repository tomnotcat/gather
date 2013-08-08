/*
 * Copyright (C) 2013 Tom Wong. All rights reserved.
 */
#include "gttocmodel.h"
#include "gtbookmark.h"
#include "gtbookmarks.h"
#include "gtdocmodel.h"
#include "gtdocpage.h"
#include "gtdocument.h"
#include "gttocdelegate.h"
#include <QtCore/QDebug>
#include <QtCore/QMimeData>

GT_BEGIN_NAMESPACE

class GtTocModelPrivate
{
    Q_DECLARE_PUBLIC(GtTocModel)

public:
    GtTocModelPrivate(GtTocModel *q);
    ~GtTocModelPrivate();

public:
    void encodeBookmarkList(const QModelIndexList &indexes,
                            QDataStream &stream) const;
    bool decodeBookmarkList(int row, int column,
                            const QModelIndex &parent,
                            QDataStream &stream);

protected:
    GtTocModel *q_ptr;
    GtDocModel *m_docModel;
    GtBookmarks *m_bookmarks;
    const QString m_mimeBookmarkList;
    const quint16 m_mimeDataMagic;
    const quint16 m_mimeDataVersion;
};

GtTocModelPrivate::GtTocModelPrivate(GtTocModel *q)
    : q_ptr(q)
    , m_docModel(0)
    , m_bookmarks(0)
    , m_mimeBookmarkList(QLatin1String("application/x-gtbookmarklist"))
    , m_mimeDataMagic(0xF7B4)
    , m_mimeDataVersion(1)
{
}

GtTocModelPrivate::~GtTocModelPrivate()
{
}

void GtTocModelPrivate::encodeBookmarkList(const QModelIndexList &indexes,
                                           QDataStream &stream) const
{
    stream << m_mimeDataMagic << m_mimeDataVersion;

    GtBookmark *bookmark;
    QModelIndexList::ConstIterator it = indexes.begin();
    for (; it != indexes.end(); ++it) {
        bookmark = static_cast<GtBookmark*>((*it).internalPointer());
        stream << *bookmark;
    }
}

bool GtTocModelPrivate::decodeBookmarkList(int row, int column,
                                           const QModelIndex &parent,
                                           QDataStream &stream)
{
    return false;
}

GtTocModel::GtTocModel(QObject *parent)
    : QAbstractItemModel(parent)
    , d_ptr(new GtTocModelPrivate(this))
{
}

GtTocModel::~GtTocModel()
{
}

GtDocModel* GtTocModel::docModel() const
{
    Q_D(const GtTocModel);
    return d->m_docModel;
}

void GtTocModel::setDocModel(GtDocModel *docModel)
{
    Q_D(GtTocModel);

    if (docModel == d->m_docModel)
        return;

    if (d->m_docModel) {
        disconnect(d->m_docModel,
                   SIGNAL(bookmarksChanged(GtBookmarks*)),
                   this,
                   SLOT(bookmarksChanged(GtBookmarks*)));

        disconnect(d->m_docModel,
                   SIGNAL(destroyed(QObject*)),
                   this,
                   SLOT(docModelDestroyed(QObject*)));
    }

    if (d->m_bookmarks) {
        disconnect(d->m_bookmarks,
                   SIGNAL(added(GtBookmark*)),
                   this,
                   SLOT(bookmarkAdded(GtBookmark*)));

        disconnect(d->m_bookmarks,
                   SIGNAL(removed(GtBookmark*)),
                   this,
                   SLOT(bookmarkRemoved(GtBookmark*)));

        disconnect(d->m_bookmarks,
                   SIGNAL(updated(GtBookmark*, int)),
                   this,
                   SLOT(bookmarkUpdated(GtBookmark*, int)));
    }

    d->m_docModel = docModel;
    d->m_bookmarks = 0;

    if (d->m_docModel) {
        d->m_bookmarks = d->m_docModel->bookmarks();

        connect(d->m_docModel,
                SIGNAL(bookmarksChanged(GtBookmarks*)),
                this,
                SLOT(bookmarksChanged(GtBookmarks*)));

        connect(d->m_docModel,
                SIGNAL(destroyed(QObject*)),
                this,
                SLOT(docModelDestroyed(QObject*)));
    }

    if (d->m_bookmarks) {
        connect(d->m_bookmarks,
                SIGNAL(added(GtBookmark*)),
                this,
                SLOT(bookmarkAdded(GtBookmark*)));

        connect(d->m_bookmarks,
                SIGNAL(removed(GtBookmark*)),
                this,
                SLOT(bookmarkRemoved(GtBookmark*)));

        connect(d->m_bookmarks,
                SIGNAL(updated(GtBookmark*, int)),
                this,
                SLOT(bookmarkUpdated(GtBookmark*, int)));
    }
}

GtBookmark* GtTocModel::bookmarkFromIndex(const QModelIndex &index) const
{
    if (!index.isValid())
        return 0;

    return static_cast<GtBookmark*>(index.internalPointer());
}

QModelIndex GtTocModel::indexFromBookmark(GtBookmark* bookmark) const
{
    if (!bookmark)
        return QModelIndex();

    return createIndex(bookmark->index(), 0, static_cast<void *>(bookmark));
}

QModelIndex GtTocModel::index(int row, int column,
                              const QModelIndex &parent) const
{
    Q_D(const GtTocModel);

    if (!hasIndex(row, column, parent))
        return QModelIndex();

    GtBookmark *node;
    if (!parent.isValid())
        node = d->m_bookmarks->root();
    else
        node = static_cast<GtBookmark*>(parent.internalPointer());

    GtBookmark *child = node->children()[row];
    if (child)
        return createIndex(row, column, child);

    return QModelIndex();
}

QModelIndex GtTocModel::parent(const QModelIndex &child) const
{
    if (!child.isValid())
        return QModelIndex();

    GtBookmark *node = static_cast<GtBookmark*>(child.internalPointer());
    GtBookmark *parent = node->parent();

    if (0 == parent)
        return QModelIndex();

    return createIndex(parent->index(), 0, static_cast<void *>(parent));
}

int GtTocModel::rowCount(const QModelIndex &parent) const
{
    Q_D(const GtTocModel);

    if (!d->m_bookmarks)
        return 0;

    if (parent.column() > 0)
        return 0;

    GtBookmark *node;
    if (!parent.isValid())
        node = d->m_bookmarks->root();
    else
        node = static_cast<GtBookmark*>(parent.internalPointer());

    return node->children().size();
}

int GtTocModel::columnCount(const QModelIndex &) const
{
    return 1;
}

QVariant GtTocModel::data(const QModelIndex &index, int role) const
{
    Q_D(const GtTocModel);

    if (!index.isValid())
        return QVariant();

    GtBookmark *node = static_cast<GtBookmark*>(index.internalPointer());
    switch (role) {
    case Qt::DisplayRole:
    case Qt::EditRole:
    case Qt::ToolTipRole:
        {
            QString title = node->title();

            if (title.isEmpty())
                title = tr("Untitled");

            return QVariant(title);
        }
        break;

    case GtTocDelegate::PageIndex:
        return QVariant(node->dest().page() + 1);

    case GtTocDelegate::PageLabel:
        {
            GtDocument *document = d->m_docModel->document();
            return QVariant(document->page(node->dest().page())->label());
        }
        break;

    default:
        break;
    }

    return QVariant();
}

bool GtTocModel::setData(const QModelIndex &index,
                         const QVariant &value, int role)
{
    GtBookmark *node = static_cast<GtBookmark*>(index.internalPointer());
    switch (role) {
    case Qt::EditRole:
        {
            QString title(value.toString());
            if (tr("Untitled") == title)
                title.clear();

            emit renameBookmark(node, title);
        }
        break;

    default:
        break;
    }

    return false;
}

Qt::ItemFlags GtTocModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::ItemIsDropEnabled;

    return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable |
            Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled;
}

Qt::DropActions GtTocModel::supportedDropActions() const
{
    return Qt::CopyAction | Qt::MoveAction;
}

QVariant GtTocModel::headerData(int, Qt::Orientation, int) const
{
    return QVariant();
}

bool GtTocModel::removeRows(int row, int count, const QModelIndex &parent)
{
    qDebug() << "remove:" << row << count;
    return false;
}

bool GtTocModel::dropMimeData(const QMimeData *data, Qt::DropAction action,
                              int row, int column, const QModelIndex &parent)
{
    Q_D(GtTocModel);

    if (!data || !(action == Qt::CopyAction || action == Qt::MoveAction))
        return false;

    if (row > rowCount(parent))
        row = rowCount(parent);

    if (row == -1)
        row = rowCount(parent);

    if (column == -1)
        column = 0;

    if (data->hasFormat(d->m_mimeBookmarkList)) {
        // decode and insert
        QByteArray encoded = data->data(d->m_mimeBookmarkList);
        QDataStream stream(&encoded, QIODevice::ReadOnly);
        return d->decodeBookmarkList(row, column, parent, stream);
    }

    return false;
}

QMimeData *GtTocModel::mimeData(const QModelIndexList &indexes) const
{
    Q_D(const GtTocModel);

    if (indexes.count() <= 0)
        return 0;

    QMimeData *data = new QMimeData();
    QByteArray encoded;
    QDataStream stream(&encoded, QIODevice::WriteOnly);
    d->encodeBookmarkList(indexes, stream);
    data->setData(d->m_mimeBookmarkList, encoded);
    return data;
}

QStringList GtTocModel::mimeTypes() const
{
    Q_D(const GtTocModel);

    QStringList types;
    types << d->m_mimeBookmarkList;
    return types;
}

void GtTocModel::bookmarksChanged(GtBookmarks *bookmarks)
{
    Q_D(GtTocModel);

    beginResetModel();

    if (d->m_bookmarks) {
        disconnect(d->m_bookmarks,
                   SIGNAL(added(GtBookmark*)),
                   this,
                   SLOT(bookmarkAdded(GtBookmark*)));

        disconnect(d->m_bookmarks,
                   SIGNAL(removed(GtBookmark*)),
                   this,
                   SLOT(bookmarkRemoved(GtBookmark*)));

        disconnect(d->m_bookmarks,
                   SIGNAL(updated(GtBookmark*, int)),
                   this,
                   SLOT(bookmarkUpdated(GtBookmark*, int)));
    }

    d->m_bookmarks = bookmarks;

    if (d->m_bookmarks) {
        connect(d->m_bookmarks,
                SIGNAL(added(GtBookmark*)),
                this,
                SLOT(bookmarkAdded(GtBookmark*)));

        connect(d->m_bookmarks,
                SIGNAL(removed(GtBookmark*)),
                this,
                SLOT(bookmarkRemoved(GtBookmark*)));

        connect(d->m_bookmarks,
                SIGNAL(updated(GtBookmark*, int)),
                this,
                SLOT(bookmarkUpdated(GtBookmark*, int)));
    }

    endResetModel();
}

void GtTocModel::bookmarkAdded(GtBookmark *)
{
    emit layoutChanged();
}

void GtTocModel::bookmarkRemoved(GtBookmark *)
{
    emit layoutChanged();
}

void GtTocModel::bookmarkUpdated(GtBookmark *bookmark, int)
{
    QModelIndex index = indexFromBookmark(bookmark);
    emit dataChanged(index, index);
}

void GtTocModel::docModelDestroyed(QObject *object)
{
    Q_D(GtTocModel);

    if (object == static_cast<QObject *>(d->m_docModel))
        setDocModel(0);
}

GT_END_NAMESPACE
