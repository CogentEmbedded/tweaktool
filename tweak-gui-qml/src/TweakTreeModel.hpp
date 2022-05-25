/**
 * @file TweakTreeModel.hpp
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
