/**
 * @file tweakuitree.cpp
 * @ingroup GUI
 *
 * @brief Tree Model for of Tweak QML Application Model.
 *
 * @copyright 2018-2021 Cogent Embedded Inc. ALL RIGHTS RESERVED.
 *
 * This file is a part of Cogent Tweak Tool feature.
 *
 * It is subject to the license terms in the LICENSE file found in the top-level
 * directory of this distribution or by request via http://cogentembedded.com
 */

#include "TweakUriTree.hpp"
#include <QRegularExpression>
#include <stdexcept>

namespace tweak2
{

void TweakUriTree::addTweak(const QStringList &path, TweakControlId id)
{
    QStringList toDo = path;
    QStringList currentPath;
    Item *current = &root;

    if (itemCacheForRemoval.contains(id))
    {
        throw std::invalid_argument("addTweak() was called with duplicate id");
    }

    while (!toDo.empty())
    {
        bool found = false;

        const QString &s = toDo.first();
        toDo.pop_front();

        currentPath.push_back(s);

        for (Item &x : current->children)
        {
            if (x.name == s)
            {
                /*.. We found a matching item, go one level deeper */
                current = &x;
                found = true;
                break;
            }
        }

        if (!found)
        {
            /*.. Create new child item here */
            Item x(*current, s, combinePathIntoUri(currentPath));

            /*.. Inform users that we created a new item */
            unsigned int index = current->children.size();
            Item *parent = current;
            emit beforeNewItem(parent, index);

            current->children.push_back(x);
            current = &*(current->children.end() - 1);

            emit afterNewItem(parent, index);
        }
    }

    /*.. current is the leaf where we should add the reference to the control */
    current->tweaks += id;

    /*.. Store path for finding the control */
    itemCacheForRemoval.insert(id, path);

    /*.. Notify interested users on new control */
    emit newTweak(id);
}

void TweakUriTree::addTweak(const QUrl &uri, TweakControlId id)
{
    addTweak(extractPathFromUri(uri), id);
}

void TweakUriTree::addTweak(const QString &uri, TweakControlId id)
{
    addTweak(QUrl(uri), id);
}

void TweakUriTree::removeTweak(TweakControlId id)
{
    auto const remainingPath = itemCacheForRemoval.find(id);
    Q_ASSERT(remainingPath != itemCacheForRemoval.end());

    removeItemInternal(root, *remainingPath, id);
}

bool TweakUriTree::removeItemInternal(TweakUriTree::Item &current,
                                      QStringList remainingPath,
                                      TweakControlId id)
{
    if (remainingPath.isEmpty())
    {
        const auto found = current.tweaks.find(id);
        Q_ASSERT(found != current.tweaks.end());
        current.tweaks.erase(found);

        itemCacheForRemoval.remove(id);
    }
    else
    {
        const QString &s = remainingPath.front();
        remainingPath.pop_front();

        auto predicate = [&s](const Item& item) {
            return item.name  == s;
        };
        auto const found = std::find_if(current.children.begin(), current.children.end(), predicate);
        Q_ASSERT(found != current.children.end());

        if (removeItemInternal(*found, remainingPath, id))
        {
            /*.. Inform the user that a item is about to be removed */
            unsigned int index = std::distance(current.children.begin(), found);
            emit beforeRemovingItem(&current, index);

            /*.. the item became empty and can be removed */
            current.children.erase(found);

            emit afterRemovingItem(&current);
        }
    }

    bool canBeRemovedFromParent =
            (current.tweaks.isEmpty() && current.children.isEmpty());
    return canBeRemovedFromParent;
}

void TweakUriTree::removeAllTweaks()
{
    while (!itemCacheForRemoval.isEmpty())
    {
        removeTweak(itemCacheForRemoval.keys().front());
    }
}

QStringList TweakUriTree::splitPathFromUri(QUrl uri)
{
    static const QRegularExpression badPathComponents =
            QRegularExpression("^[ ]*$");

    if (!uri.isValid())
        return QStringList();

    QStringList path = uri.path().split("/");

    /*.. Fold incorrect components */
    const auto bad = path.filter(badPathComponents);
    foreach (const QString &b, bad)
    {
        path.removeAll(b);
    }

    return path;
}

QStringList TweakUriTree::extractPathFromUri(QUrl uri)
{
    QStringList path = splitPathFromUri(uri);

    /*.. Remove last element, which must be tweak individual name */
    path.pop_back();

    return path;
}

QUrl TweakUriTree::combinePathIntoUri(const QStringList &path)
{
    return QUrl("/" + path.join("/"));
}

const QSet<TweakControlId> &
TweakUriTree::tweaksForPathInternal(const Item &current,
                                    QStringList remainingPath) const
{
    if (remainingPath.isEmpty())
    {
        return current.tweaks;
    }
    else
    {
        const QString &s = remainingPath.first();
        remainingPath.pop_front();

        auto predicate = [&s](const Item& item) {
            return item.name  == s;
        };
        auto const found = std::find_if(current.children.begin(), current.children.end(), predicate);

        if (found != current.children.end())
        {
            return tweaksForPathInternal(*found, remainingPath);
        }
        else
        {
            /*.. inexistent item requested */
            throw std::invalid_argument("remainingPath");
        }
    }
}

const void *TweakUriTree::itemForPathInternal(const TweakUriTree::Item &current, QStringList remainingPath) const
{
    if (remainingPath.isEmpty())
    {
        return &current;
    }
    else
    {
        const QString &s = remainingPath.first();
        remainingPath.pop_front();

        auto predicate = [&s](const Item& item) {
            return item.name  == s;
        };
        auto const found = std::find_if(current.children.begin(), current.children.end(), predicate);

        if (found != current.children.end())
        {
            return itemForPathInternal(*found, remainingPath);
        }
        else
        {
            /*.. inexistent item requested */
            return nullptr;
        }
    }
}

const QSet<TweakControlId> &TweakUriTree::tweaks(const QUrl &path) const
{
    QStringList p = splitPathFromUri(path);
    return tweaksForPathInternal(root, p);
}

const QSet<TweakControlId> &TweakUriTree::tweaks(const Item *item) const
{
    return item->tweaks;
}

const void *TweakUriTree::rootItem() const { return &root; }

unsigned int TweakUriTree::childCount(const void *const item) const
{
    if (item == nullptr)
    {
        throw std::invalid_argument("item");
    }

    const Item &n = *static_cast<const Item *const>(item);

    return n.children.size();
}

const void *TweakUriTree::child(const void *const item, int index) const
{
    if (item == nullptr)
    {
        throw std::invalid_argument("item");
    }

    const Item &n = *static_cast<const Item *const>(item);
    const auto found = n.children.begin() + index;

    /*.. found is always valid */
    return &*found;
}

const void *TweakUriTree::parentItem(const void *const item) const
{
    if (item == nullptr)
    {
        throw std::invalid_argument("item");
    }

    const Item &n = *static_cast<const Item *>(item);

    return &n.parent;
}

unsigned int TweakUriTree::childIndex(const void *const parent,
                                      const void *const item) const
{
    if (parent == nullptr)
    {
        throw std::invalid_argument("parent");
    }
    if (item == nullptr)
    {
        throw std::invalid_argument("item");
    }

    const Item &p = *static_cast<const Item *>(parent);
    const Item &n = *static_cast<const Item *>(item);

    const QString s = n.name;
    auto predicate = [&s](const Item& item) {
        return item.name  == s;
    };
    auto const found = std::find_if(p.children.begin(), p.children.end(), predicate);

    if (found == p.children.end())
    {
        /*.. item is not a child of parent */
        throw std::invalid_argument("item");
    }

    int dist = std::distance(p.children.constBegin(), found);
    Q_ASSERT(dist >= 0);

    return dist;
}

QString TweakUriTree::itemDisplayName(const void *const item) const
{
    if (item == nullptr)
    {
        throw std::invalid_argument("item");
    }

    const Item &n = *static_cast<const Item *>(item);

    QString name = n.name;
    Q_ASSERT(!name.isEmpty());

    return name;
}

QUrl TweakUriTree::itemUri(const void * const item) const
{
    if (item == nullptr)
    {
        throw std::invalid_argument("item");
    }

    const Item &n = *static_cast<const Item *>(item);

    QUrl uri = n.path;
    Q_ASSERT(!uri.isEmpty());

    return uri;
}

const void *TweakUriTree::itemByUri(const QUrl path) const
{
    QStringList p = splitPathFromUri(path);
    return itemForPathInternal(root, p);
}

TweakUriTree::Item TweakUriTree::Item::root()
{
    Item r(r, "<root>", QUrl("/"));

    return r;
}

} // namespace tweak2
