
/**
 * @file TweakUriTree.hpp
 * @ingroup GUI
 *
 * @brief Tree Model for of Tweak QML Application Model.
 *
 * @copyright 2020-2022 Cogent Embedded, Inc. ALL RIGHTS RESERVED.
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

#ifndef TWEAK_URI_TREE_H_INCLUDED
#define TWEAK_URI_TREE_H_INCLUDED

#include "tweak2/types.h"

#include <QMap>
#include <QObject>
#include <QSet>
#include <QUrl>
#include <QtGlobal>

namespace tweak2
{
using ConnectionId = qint64;

constexpr ConnectionId InvalidClientConnectionId = -1;

/**
 * @brief Uniqie identifier of the control.
 */
struct TweakControlId
{
public:
    /**
     * @brief Connection Id to separate individual tweakId which can be
     *        duplicated in different connections.
     * @note  64-bit data type is used to have nice alignment.
     */
    ConnectionId connectionId;

    /**
     * @brief Tweak Id within the connection.
     */
    tweak_id tweakId;

    TweakControlId(ConnectionId connectionId, tweak_id tweakId)
        : connectionId(connectionId), tweakId(tweakId)
    {
    }

    TweakControlId() : connectionId(-1), tweakId(TWEAK_INVALID_ID) {}

    bool operator==(const TweakControlId &b) const
    {
        return connectionId == b.connectionId && tweakId == b.tweakId;
    }

    /**
     * @brief @c true if both connectionId and tweakId are valid.
     */
    bool isValid() const
    {
        return connectionId >= InvalidClientConnectionId && tweakId != TWEAK_INVALID_ID;
    }
};

/**
 * @see https://www.kdab.com/how-to-declare-a-qhash-overload/
 */
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
using qhash_result_t = size_t;
#else
using qhash_result_t = uint;
#endif

Q_DECL_PURE_FUNCTION inline qhash_result_t
qHash(const tweak2::TweakControlId &key, qhash_result_t seed = 0) noexcept
{
    QtPrivate::QHashCombine hash;

    seed = hash(seed, key.connectionId);
    seed = hash(seed, key.tweakId);

    return seed;
}

/**
 * @brief
 *
 * @details
 * The tree of controls is build from items. Parent item is stored the the tree
 * itself @ref root. Each addition of a item can causes:
 * 1. URI parsing and splitting.
 * 2. Tree walk, from root.
 * 3. Creation of new items if needed.
 * 4. The item itself is added to its corresponding leaf.
 *
 * Item names are case-insensitive.
 *
 * This class is not thread-safe, it must be protected if called from multiple
 * threads.
 *
 * @todo Cleanup naming, use 'tweak' instead of 'item'.
 */
class TweakUriTree : public QObject
{
    Q_OBJECT

private:
    /**
     * @brief A item in the tree of names.
     *
     * @details Each item can be a branch or a leaf.
     * Branch contains 1..* collection of other branches and/or leafs.
     * Leaf contains a single tweak.
     */
    struct Item
    {
        /**
         * @brief Item name. Cannot be empty string or contan '/' characters.
         */
        QString name;

        /**
         * @brief Path to the item. Useful for caching.
         */
        QUrl path;

        /**
         * @brief Reference to the parent item.
         * @details Items are always deleted children first, so the reference
         * cannot become dangling.
         */
        Item &parent;

        /**
         * @brief Children.
         *
         */
        QList<Item> children;

        /**
         * @brief List of tweaks displayed in the tree item.
         */
        QSet<TweakControlId> tweaks;

        /**
         * @brief Create new item.
         * @param parent Reference to the parent item.
         * @param name Name of the item.
         * @param path Full path to the item.
         */
        Item(Item &parent, const QString &name, const QUrl &path)
            : name(name), path(path), parent(parent)
        { }

        /**
         * @note Default constructor is deleted on purpose.
         * Items are immutable.
         */
        Item() = delete;

        /**
         * @note Сopy is only possible on empty items.
         */
        Item(const Item &other)
            : name(other.name), path(other.path), parent(other.parent)
        {
            Q_ASSERT(other.children.isEmpty());
            Q_ASSERT(other.tweaks.isEmpty());
        }

private:
        Item(const QString &name, const QUrl &path)
            : name(name), path(path), parent(*this)
        {}
public:
        /**
         * @note Assignment is only possible on empty items.
         */
        Item &operator=(const Item &other)
        {
            Q_ASSERT(children.isEmpty());
            Q_ASSERT(tweaks.isEmpty());

            Q_ASSERT(other.children.isEmpty());
            Q_ASSERT(other.tweaks.isEmpty());

            if (this != &other)
            {
                name = other.name;
                path = other.path;
                parent = other.parent;
            }

            return *this;
        }

        ~Item()
        {
            /*.. A item can only be deleted if it is empty */
            Q_ASSERT(children.isEmpty());
            Q_ASSERT(tweaks.isEmpty());
        }

        /**
         * @brief Root item of the tree.
         * @details Root item must be invisible to the user in normal cases
         */
        static Item root();
    };

private:

    /**
     * @brief Root item that is always present.
     */
    Item root = Item::root();

    /**
     * @brief Cache of items for fast deleting.
     */
    QHash<TweakControlId, QStringList> itemCacheForRemoval;

    /**
     * @brief Remove a item downstream of @p current with remaining path @p
     * remainingPath.
     * @return @c true, if the item is empty and can be removed. @c false
     * otherwise.
     */
    bool removeItemInternal(Item &current, QStringList remainingPath,
                            TweakControlId id);

    /**
     * @brief List tweaks for path tweaksForPathInternal
     * @param current
     * @param remainingPath
     * @return
     */
    const QSet<TweakControlId> &
    tweaksForPathInternal(const Item &current, QStringList remainingPath) const;

    /**
     * @brief Locate the item for the @p remainingPath.
     * @param current Item to start with.
     * @param remainingPath Remaining portion of the path. @p current shall be returned of @p remainingPAth is empty.
     */
    const void* itemForPathInternal(const Item &current, QStringList remainingPath) const;

    /**
     * @brief Split item URI into individual path components.
     * @details Each path component cannot be empty string, such components are
     * folded.
     * @param uri Server-supplied URI.
     * @return URI components.
     */
    static QStringList splitPathFromUri(QUrl uri);

    /**
     * @brief Extract path to the control from URI by removing last element from
     * the path.
     * @details Each path component cannot be empty string, such components are
     * folded.
     * @param uri Server-supplied URI.
     * @return Path components.
     */
    static QStringList extractPathFromUri(QUrl uri);

    /**
     * @brief Combine path into an URI.
     * @param path Path to combine.
     * @return Combined URI in standard format.
     */
    static QUrl combinePathIntoUri(const QStringList &path);

    /**
     * @brief Add a tweak to the tree, creating the necessary path elements.
     * @param path Path to the tweak, separated into components in advance.
     *             Empty path will add the tweak to the root item.
     * @param id Tweak Id.
     */
    void addTweak(const QStringList &path, TweakControlId id);

signals: /** @subsection API for tweak side */

    /**
     * @brief New tweak was added to the tree.
     * @param id Tweak Id.
     */
    void newTweak(TweakControlId id);

    /**
     * @brief Before new item that was added to the tree.
     *
     * @param parent Parent of the item that is about to be added.
     * @param index Index of the item in the parent.
     */
    void beforeNewItem(const void *parent, unsigned int index);

    /**
     * @brief After new item that was added to the tree.
     *
     * @param parent Parent of the item that was added.
     * @param index Index of the item in the parent.
     */
    void afterNewItem(const void *parent, unsigned int index);

    /**
     * @brief One item is about to be removed.
     *
     * @param parent Parent of the item that will be removed from the tree.
     * @param index Index of the item in the parent.
     */
    void beforeRemovingItem(const void *parent, unsigned int index);

    /**
     * @brief One item was removed.
     *
     * @param parent Parent of the item that was removed from the tree.
     */
    void afterRemovingItem(const void *parent);

public:
    /**
     * @brief Constructs an object with parent object @p parent.
     */
    TweakUriTree(QObject *parent = nullptr) : QObject(parent) {}

    /**
     * @brief Add a tweak to the tree, creating the necessary path elements.
     * @param uri Full path to the tweak. Duplicated and empty paths are not
     * allowed.
     * @param id Tweak Id.
     */
    void addTweak(const QUrl &uri, TweakControlId id);

    /**
     * @brief Add a tweak to the tree, creating the necessary path elements.
     * @param uri Full path to the tweak. Duplicated and empty paths are not
     * allowed.
     * @param id Tweak Id.
     */
    void addTweak(const QString &uri, TweakControlId id);

    /**
     * @brief Remove one tweak from the tree.
     * @details This function will remove all empty items that are left after
     * deleting the subject item.
     *
     * @param id Tweak Id.
     */
    void removeTweak(TweakControlId id);

    /**
     * @brief Remove all tweaks from the tree.
     * @details This method is typically called from destructor but may be
     * useful in corner cases such as connection drop handling.
     */
    void removeAllTweaks();

public: /** @subsection API for GUI */
    /**
     * @brief List of controls for tree path @p path.
     * @param path Path to the item in the tree. Can be empty, root item tweaks
     * are returned in this case .
     * @return Set of tweaks.
     */
    const QSet<TweakControlId> &tweaks(const QUrl &path) const;

    /**
     * @brief List of tweaks for the item, not including tweaks from child
     * items.
     * @param item Item to return tweaks for. Not null.
     * @return Set of tweaks.
     */
    const QSet<TweakControlId> &tweaks(const Item *item) const;

    /**
     * @brief Pointer to the root item.
     * @details This function is define for optimization since the same result
     * can be obtained through @ref child("", 0);
     */
    const void *rootItem() const;

    /**
     * @brief Number of children for a particular item.
     * @param item Item to inspect.
     * @return Number of children, greater or equal to zero.
     */
    unsigned int childCount(const void* item) const;

    /**
     * @brief A child of the given @p item with the @p index.
     * @details This function triggers an exception if invalid @p item or @p
     * index are supplied.
     * @return Item. Not null.
     */
    const void *child(const void* item, int index) const;

    /**
     * @brief A parent of the given @p item.
     * @param item A valid item to seek parent for.
     * @return Parent item. Not null.
     */
    const void *parentItem(const void * item) const;

    /**
     * @brief Checks if @p item is the root item of the tree.
     *
     * Behaviour is undefined if @p item does not belong to the tree.
     *
     * @param item Item to check.
     * @return @c true if @p item is the root of the tree.
     */
    bool isRootItem(const void * item) const
    {
        return item == rootItem();
    }

    /**
     * @brief Index of child @p item in the @p parent item.
     * @param parent Parent item, can be @ref rootItem.
     * @param item Child item, cannot be @ref rootItem.
     *
     * @return Item index. The value must be >=0
     *
     * @note The function throws @ref std::invalid_argument exception in case
     *       the @p item is not a child of @p parent.
     */
    unsigned int childIndex(const void * parent,
                            const void * item) const;

    /**
     * @brief User-visible name of the item.
     * @return Non-empty string.
     */
    QString itemDisplayName(const void * item) const;

    /**
     * @brief Uri of the item.
     * @return Non-empty uri.
     */
    QUrl itemUri(const void * item) const;

    /**
     * @brief Find an item by its uri.
     * @param path Item uri.
     */
    const void* itemByUri(const QUrl path) const;

    virtual ~TweakUriTree() override { removeAllTweaks(); }
};

} // namespace tweak2

Q_DECLARE_METATYPE(tweak2::TweakControlId)
Q_DECLARE_METATYPE(tweak_id)

#endif /* TWEAK_URI_TREE_H_INCLUDED */
