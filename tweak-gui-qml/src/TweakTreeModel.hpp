/**
 * @file TweakTreeModel.hpp
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

#ifndef TWEAKTREEMODEL_HPP
#define TWEAKTREEMODEL_HPP

#include "TweakUriTree.hpp"
#include <QAbstractItemModel>

namespace tweak2
{

class TweakTreeModel : public QAbstractItemModel
{
    Q_OBJECT
    Q_DISABLE_COPY(TweakTreeModel)

public:
    enum class ItemType {
        Leaf,
        Connection,
        Favorites,
        Root,
    };
    Q_ENUM(ItemType)

    TweakTreeModel(QObject *parent = Q_NULLPTR);

    int rowCount(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;
    int columnCount(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;
    QVariant data(const QModelIndex &index, int role) const Q_DECL_OVERRIDE;
    QModelIndex index(int row, int column = 0, const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;
    QHash<int, QByteArray> roleNames() const Q_DECL_OVERRIDE;

    enum Roles
    {
        TreeItemRole = Qt::UserRole + 1,
    };

    void newItem(QString uri, TweakControlId tweakControlId);

    void itemRemoved(TweakControlId tweakControlId);

public slots:
    QString selectionToRegExp(const QModelIndex &index) const;
    QString indexToUri(const QModelIndex &index) const;
    QModelIndex indexByUri(const QString uri) const;
    QModelIndex itemParent(const QModelIndex index) const;

private slots:
    void beforeNewItemAddedToTree(const void *item, unsigned int index);
    void afterNewItemAddedToTree(const void *item, unsigned int index);

    void beforeRemovingItemFromTree(const void *parent, unsigned int index);
    void afterRemovingItemFromTree(const void *parent);

private:
    TweakUriTree tree;

    QModelIndex parent(const QModelIndex &index) const Q_DECL_OVERRIDE;

    QModelIndex itemIndex(const void* item) const;
};

} // namespace tweak2

#endif // TWEAKTREEMODEL_HPP
