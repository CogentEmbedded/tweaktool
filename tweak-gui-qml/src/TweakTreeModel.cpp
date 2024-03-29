/**
 * @file TweakTreeModel.cpp
 * @ingroup GUI
 *
 * @brief Tree Model for of Tweak QML Application Model.
 *
 * @copyright 2020-2023 Cogent Embedded, Inc. ALL RIGHTS RESERVED.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include "TweakTreeModel.hpp"

#include <QLinkedList>
#include <QtDebug>

namespace tweak2
{

TweakTreeModel::TweakTreeModel(QObject *parent) : QAbstractItemModel(parent)
{
    connect(&tree, &TweakUriTree::beforeNewItem, this,
            &TweakTreeModel::beforeNewItemAddedToTree, Qt::DirectConnection);
    connect(&tree, &TweakUriTree::afterNewItem, this,
            &TweakTreeModel::afterNewItemAddedToTree, Qt::DirectConnection);
    connect(&tree, &TweakUriTree::beforeRemovingItem, this,
            &TweakTreeModel::beforeRemovingItemFromTree, Qt::DirectConnection);
    connect(&tree, &TweakUriTree::afterRemovingItem, this,
            &TweakTreeModel::afterRemovingItemFromTree, Qt::DirectConnection);

    tree.addTweak("/Favorites/*", {-1, TWEAK_INVALID_ID});
}

int TweakTreeModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid() && parent.column() != 0)
    {
        return 0;
    }

    const void *parentItem = tree.rootItem();
    if (parent.isValid())
    {
        parentItem = parent.internalPointer();
    }

    int count;
    if (parentItem == nullptr)
    {
        count = 1;
    }
    else
    {
        count = static_cast<int>(tree.childCount(parentItem));
    }

    qDebug() << "rowCount "
             << "parent="
             << (parentItem ? tree.itemUri(parentItem) : QUrl("null"))
             << " count=" << count;

    return count;
}

int TweakTreeModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return 1;
}

QVariant TweakTreeModel::data(const QModelIndex &index, int role) const
{
    QString itemName;
    ItemType itemType;

    if (index.isValid())
    {
        const void *Item = index.internalPointer();
        if (Item)
        {
            itemName = tree.itemDisplayName(Item);
            const QUrl itemUrl = tree.itemUri(Item);

            if (itemUrl.matches(QUrl("/Favorites"),
                                QUrl::UrlFormattingOption::None))
            {
                itemType = ItemType::Favorites;
            }
            else if (index.parent() == QModelIndex())
            {
                itemType = ItemType::Connection;
            }
            else
            {
                /*.. normal leaves */
                itemType = ItemType::Leaf;
            }
        }
        else
        {
            qWarning() << "Invalid item supplied to data()";
        }
    }
    else
    {
        itemType = ItemType::Root;
    }

    switch (role)
    {
    case TreeItemRole:
        return QVariantMap({
            {"itemType", QVariant::fromValue(itemType)},
            {"name", itemName},
        });

    case NameRole:
        return itemName;

    default:
        qWarning() << "Invalid role requested from data(): " << role;
        return QVariant();
    }
}

QModelIndex TweakTreeModel::index(int row, int column,
                                  const QModelIndex &parent) const
{
    if (parent.isValid() && parent.column() != 0)
    {
        /*.. non-zero columns are invalid in the tree */
        return QModelIndex();
    }

    const void *parentItem = tree.rootItem();
    if (parent.isValid() && (parent.internalPointer() != nullptr))
    {
        parentItem = parent.internalPointer();
    }

    const void *childItem = nullptr;
    int count = tree.childCount(parentItem);
    if (row < count)
    {
        childItem = tree.child(parentItem, row);
    }
    else
    {
        /*.. TODO: Sometimes invalid indicies are produced, this is very bad */
        qWarning() << "Invalid child index for " << tree.itemUri(parentItem)
                   << " :" << row << " max: " << count;
    }

    qDebug() << "index"
             << "childItem="
             << (childItem ? tree.itemUri(childItem) : QUrl("null"))
             << "row=" << row << "column=" << column;

    if (childItem != nullptr)
    {
        /*.. const_cast<>() is a workaround for legacy API in Qt. The actual
         *   pointer and data never change later */
        return createIndex(row, column, const_cast<void *>(childItem));
    }
    else
    {
        return QModelIndex();
    }
}

QHash<int, QByteArray> TweakTreeModel::roleNames() const
{
    return {
        {NameRole, "name"},
        {TreeItemRole, "item"},
    };
}

void TweakTreeModel::newItem(QString uri, TweakControlId tweakControlId)
{
    tree.addTweak(uri, tweakControlId);
}

void TweakTreeModel::itemRemoved(TweakControlId tweakControlId)
{
    tree.removeTweak(tweakControlId);
}

QString TweakTreeModel::selectionToRegExp(const QModelIndex &index) const
{
    const void *Item = index.internalPointer();

    if (Item)
    {
        /*.. generate RegEx for selecting common uri prefix */
        const QUrl uri = tree.itemUri(Item);
        QString path = uri.path();
        path = path.endsWith("/") ? path : path + "/";

        QString regexp = "^" + QRegExp::escape(path);

        return regexp;
    }
    else
    {
        /*.. Show all otherwise */
        return ".";
    }
}

QString TweakTreeModel::indexToUri(const QModelIndex &index) const
{
    const void *Item = index.internalPointer();

    if (Item)
    {
        const QUrl uri = tree.itemUri(Item);
        return uri.toString();
    }
    else
    {
        return "/";
    }
}

QModelIndex TweakTreeModel::indexByUri(const QString uri) const
{
    const void *item = tree.itemByUri(uri);

    if (item != nullptr)
    {
        return itemIndex(item);
    }
    else
    {
        return QModelIndex();
    }
}

QModelIndex TweakTreeModel::itemParent(const QModelIndex index) const
{
    return parent(index);
}

void TweakTreeModel::beforeNewItemAddedToTree(const void *parent,
                                              unsigned int index)
{
    qDebug() << "Before adding item: parent=" << tree.itemUri(parent)
             << ", index=" << index;
    QModelIndex parentIndex = itemIndex(parent);
    beginInsertRows(parentIndex, index, index);
}

void TweakTreeModel::afterNewItemAddedToTree(const void *parent,
                                             unsigned int index)
{
    qDebug() << "After adding item: parent=" << tree.itemUri(parent)
             << ", index=" << index;
    endInsertRows();
}

void TweakTreeModel::beforeRemovingItemFromTree(const void *parent,
                                                unsigned int index)
{
    qDebug() << "Removing item: parent=" << tree.itemUri(parent)
             << ", index=" << index;
    QModelIndex parentIndex = itemIndex(parent);
    beginRemoveRows(parentIndex, index, index);
}

void TweakTreeModel::afterRemovingItemFromTree(const void *parent)
{
    Q_UNUSED(parent);
    endRemoveRows();
}

QModelIndex TweakTreeModel::parent(const QModelIndex &index) const
{
    const void *childItem = index.internalPointer();
    if (childItem == nullptr)
    {
        return QModelIndex();
    }

    if (!index.isValid())
    {
        return QModelIndex();
    }

    Q_ASSERT(!tree.isRootItem(childItem));
    const void *parentItem = tree.parentItem(childItem);

    qDebug() << "parent"
             << " childItem="
             << (childItem ? tree.itemUri(childItem) : QUrl("null"))
             << " parentItem="
             << (parentItem ? tree.itemUri(parentItem) : QUrl("null"));

    if (parentItem == tree.rootItem())
    {
        return QModelIndex();
    }

    const void *parentOfParentItem = tree.parentItem(parentItem);
    int row = static_cast<int>(tree.childIndex(parentOfParentItem, parentItem));

    qDebug() << "parent"
             << " row=" << row;

    /*.. const_cast<>() is a workaround for legacy API in Qt. The actual pointer
     * and data never change later */
    return createIndex(row, 0, const_cast<void *>(parentItem));
}

QModelIndex TweakTreeModel::itemIndex(const void *item) const
{
    Q_ASSERT(item);

    if (tree.isRootItem(item))
    {
        return QModelIndex();
    }
    else
    {
        const void *parent = tree.parentItem(item);
        unsigned int row = tree.childIndex(parent, item);
        return index(row, 0, itemIndex(parent));
    }
}

} // namespace tweak2
