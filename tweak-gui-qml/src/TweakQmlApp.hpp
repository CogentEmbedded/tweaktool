/**
 * @file tweakqmlapp.hpp
 * @ingroup GUI
 *
 * @brief Tweak QML Application Model.
 *
 * @copyright 2018-2021 Cogent Embedded Inc. ALL RIGHTS RESERVED.
 *
 * This file is a part of Cogent Tweak Tool feature.
 *
 * It is subject to the license terms in the LICENSE file found in the top-level
 * directory of this distribution or by request via http://cogentembedded.com
 */

#ifndef TWEAK_QML_APP_H_INCLUDED
#define TWEAK_QML_APP_H_INCLUDED

#include "TweakUriTree.hpp"
#include "TweakMetadataParser.hpp"

#include <QAbstractListModel>
#include <QJSEngine>
#include <QThread>

#include <QHash>
#include <QSharedPointer>

namespace tweak2
{

class TweakApplicationPrivate;
class TweakTreeModel;

class TweakApplication : public QAbstractListModel
{
    Q_OBJECT

private:
    Q_DECLARE_PRIVATE(TweakApplication)

    TweakApplicationPrivate *const d_ptr;

public:
    explicit TweakApplication(QObject *parent = Q_NULLPTR,
                              Qt::ConnectionType connectionType = Qt::QueuedConnection);

    int rowCount(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;
    QVariant data(const QModelIndex &index, int role) const Q_DECL_OVERRIDE;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) Q_DECL_OVERRIDE;
    QHash<int, QByteArray> roleNames() const Q_DECL_OVERRIDE;
    Qt::ItemFlags flags(const QModelIndex &index) const Q_DECL_OVERRIDE;

    enum Roles
    {
        UriRole = Qt::UserRole + 1,
        ValueRole,
        DefaultValueRole,
        DescriptionRole,
        MetaRole,
        isFavoriteRole,
    };

signals:
    void treeChanged();
    void metadataParserChanged();
    void favoritesRegExChanged();

public slots:
    void currentValueChanged(quint64 connection_id, quint64 tweak_id);
    void statusChanged(quint64 connection_id, bool is_connected);
    void newItem(quint64 connection_id, quint64 tweak_id, QString uri);
    void itemRemoved(quint64 connection_id, quint64 tweak_id);

public:
    Q_PROPERTY(tweak2::TweakTreeModel *tree READ getTreeModel NOTIFY treeChanged)
    Q_PROPERTY(tweak2::TweakMetadataParser *metadataParser READ getMetadataParser NOTIFY metadataParserChanged)
    Q_PROPERTY(QString favoritesRegEx READ getfavoritesRegEx NOTIFY favoritesRegExChanged)

public slots:
    QVariant get(ConnectionId connectionId, const QUrl &uri) const;
    QVariant get(ConnectionId connectionId, const QString &path) const;

    qint64 addClient(QString name, QString contextType, QString params, QString uri);
    void removeClient(qint64 clientConnectionId);

    void addFavorite(const QString &uri);
    void removeFavorite(const QString &uri);
    void clearFavorites();
    bool isFavorite(const QString &uri) const;

private:
    TweakTreeModel *getTreeModel();
    TweakMetadataParser *getMetadataParser();
    QString getfavoritesRegEx() const;

    QJSEngine scriptEngine;

    QSet<QString> favorites;

    using MetadataCacheKey = std::pair<tweak_variant_type, QString>;
    using MetadataCacheItem = QSharedPointer<TweakMetadata>;

    mutable QHash<MetadataCacheKey, MetadataCacheItem> metadataCache;
};

} // namespace tweak2

#endif /* TWEAK_QML_APP_H_INCLUDED */
